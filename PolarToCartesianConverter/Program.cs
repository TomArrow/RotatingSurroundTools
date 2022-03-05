using System;
using System.Collections.Generic;
using System.IO;
using System.Numerics;
using System.Text.Json;

namespace PolarToCartesianConverter
{
    class Program
    {
        static void Main(string[] args)
        {
            if(args.Length == 0)
            {
                return;
            }

            JsonSerializerOptions opt = new JsonSerializerOptions();
            opt.NumberHandling = System.Text.Json.Serialization.JsonNumberHandling.AllowReadingFromString;
            CommonDefinitions commonDefinitions = JsonSerializer.Deserialize<CommonDefinitions>(File.ReadAllText("common_definitions.json"), opt);

            string audioPackFormatId = args[0];

            string outputFilename = audioPackFormatId + ".csv";
            if (File.Exists(outputFilename))
            {
                return;
            }

            Audiopackformat[] packFormats = commonDefinitions.coreMetadata.format.audioFormatExtended.audioPackFormat;

            Audiopackformat chosenPackFormat = null;

            foreach(Audiopackformat packFormat in packFormats)
            {
                if(packFormat.audioPackFormatID == audioPackFormatId)
                {
                    chosenPackFormat = packFormat;
                    break;
                }
            }

            if (chosenPackFormat == null) return;

            Audiochannelformat[] channelFormats = commonDefinitions.coreMetadata.format.audioFormatExtended.audioChannelFormat;

            List<string> info = new List<string>();
            foreach (string channelRef in chosenPackFormat.audioChannelFormatIDRef)
            {
                // Find corresponding channel
                foreach(Audiochannelformat chanFormat in channelFormats)
                {
                    if(chanFormat.audioChannelFormatID == channelRef)
                    {
                        double azimuth=0, elevation=0, distance=0;
                        foreach(Position pos in chanFormat.audioBlockFormat.position)
                        {
                            switch (pos.coordinate)
                            {
                                case "azimuth":
                                    azimuth = pos.text;
                                    break;
                                case "elevation":
                                    elevation = pos.text;
                                    break;
                                case "distance":
                                    distance = pos.text;
                                    break;
                                default:break;
                            }

                        }

                        Console.WriteLine($"Azimuth {azimuth}, elevation {elevation}, distance {distance} ");
                        Vector3 cartesian = ConversionCode.PolarToCartesian(azimuth, elevation, distance);
                        Console.WriteLine(cartesian);
                        info.Add( $"{cartesian.X},{cartesian.Y},{cartesian.Z}");
                    }
                }
            }
            File.WriteAllLines(outputFilename, info);

            //Console.WriteLine("Hello World!");

            Console.ReadKey();
        }


    }
}
