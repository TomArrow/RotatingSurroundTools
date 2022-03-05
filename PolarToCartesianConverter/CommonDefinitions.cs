
using System.Collections.Generic;

public class CommonDefinitions
{
    public Coremetadata coreMetadata { get; set; }
}

public class Coremetadata
{
    public Format format { get; set; }
}

public class Format
{
    public Audioformatextended audioFormatExtended { get; set; }
}

public class Audioformatextended
{
    public Audiopackformat[] audioPackFormat { get; set; }
    public Audiochannelformat[] audioChannelFormat { get; set; }
    public Audiostreamformat[] audioStreamFormat { get; set; }
    public Audiotrackformat[] audioTrackFormat { get; set; }
}

public class Audiopackformat
{
    public string audioPackFormatID { get; set; }
    public string audioPackFormatName { get; set; }
    public string typeLabel { get; set; }
    public string typeDefinition { get; set; }
    public List<string> audioChannelFormatIDRef { get; set; }
    public string audioPackFormatIDRef { get; set; }
}

public class Audiochannelformat
{
    public string audioChannelFormatID { get; set; }
    public string audioChannelFormatName { get; set; }
    public string typeLabel { get; set; }
    public string typeDefinition { get; set; }
    public Audioblockformat audioBlockFormat { get; set; }
    public Frequency frequency { get; set; }
}

public class Audioblockformat
{
    public string audioBlockFormatID { get; set; }
    public string speakerLabel { get; set; }
    public Position[] position { get; set; }
    public string degree { get; set; }
    public string order { get; set; }
    public string normalization { get; set; }
}

public class Position
{
    public string coordinate { get; set; }
    public double text { get; set; }
    public string screenEdgeLock { get; set; }
}

public class Frequency
{
    public string typeDefinition { get; set; }
    public string text { get; set; }
}

public class Audiostreamformat
{
    public string audioStreamFormatID { get; set; }
    public string audioStreamFormatName { get; set; }
    public string formatLabel { get; set; }
    public string formatDefinition { get; set; }
    public string audioChannelFormatIDRef { get; set; }
    public string audioTrackFormatIDRef { get; set; }
}

public class Audiotrackformat
{
    public string audioTrackFormatID { get; set; }
    public string audioTrackFormatName { get; set; }
    public string formatLabel { get; set; }
    public string formatDefinition { get; set; }
    public string audioStreamFormatIDRef { get; set; }
}
