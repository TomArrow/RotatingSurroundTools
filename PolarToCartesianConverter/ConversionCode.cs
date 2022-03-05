/*
 * Adapted from: https://github.com/ebu/ebu_adm_renderer
 * 
 * Copyright (c) 2018-2019 EBU ADM Renderer Authors
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below) provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Numerics;

namespace PolarToCartesianConverter
{
    static class ConversionCode
    {

        const double elevationTop = 30.0f;
        const double elevationTopTilde = 45.0f;

        struct Mapping
        {
            public double angle;
            public Vector3 position;
        }

        static Mapping[] mappings = new Mapping[] {
            new Mapping{ angle=0,position= new Vector3{X=0,Y=1,Z=0 } },
            new Mapping{ angle=-30,position= new Vector3{X=1,Y=1,Z=0 } },
            new Mapping{ angle=-110,position= new Vector3{X=1,Y=-1,Z=0 } },
            new Mapping{ angle=110,position= new Vector3{X=-1,Y=-1,Z=0 } },
            new Mapping{ angle=30,position= new Vector3{X=-1,Y=1,Z=0 } },
        };

        static public Vector3 PolarToCartesian(double azimuth, double elevation, double distance)
        {
            Vector3 retVal = new Vector3();
            double r_xy,z;
            if (Math.Abs(elevation) > elevationTop)
            {
                double el_tilde = elevationTopTilde + (90.0 - elevationTopTilde) * (Math.Abs(elevation) - elevationTop) / (90.0 - elevationTop);
                z = distance * Math.Sign(elevation);
                r_xy = distance * Math.Tan(ToRadians(90 - el_tilde)); 
            }
            else
            {
                double el_tilde = elevationTopTilde * elevation / elevationTop;
                z = Math.Tan(ToRadians(el_tilde)) * distance;
                r_xy = distance;
            }

            Mapping[] foundMappings = FindSector(azimuth);
            double left_az = foundMappings[0].angle, right_az = foundMappings[1].angle;
            Vector3 left_pos = foundMappings[0].position, right_pos = foundMappings[1].position;

            double rel_az = RelativeAngle(right_az, azimuth);
            double rel_left_az = RelativeAngle(right_az, left_az);

            double p = MapAzimuthToLinear(rel_left_az, right_az, rel_az);

            Vector3 tmp = (float)r_xy * (left_pos + (right_pos - left_pos) * (float)p);
            retVal = tmp;
            retVal.Z = (float)z;

            return retVal;
        }

        static Mapping[] FindSector(double azimuth)
        {
            for(int i=0;i<mappings.Length;i++)
            {
                int j = (i + 1) % mappings.Length;

                if (InsideAngleRange(azimuth, mappings[j].angle, mappings[i].angle))
                {
                    return new Mapping[] { mappings[i], mappings[j] };
                }
            }
            return null; // Should not happen
        }

        static bool InsideAngleRange(double x, double start, double end, double tol = 0.0)
        {
            while (end - 360.0 > start)
                end -= 360.0;
            while (end < start)
                end += 360.0;

            double start_tol = start - tol;
            while (x - 360.0 >= start_tol)
                x -= 360.0;
            while (x < start_tol)
                x += 360.0;

            return x <= end + tol;
        }

        static double RelativeAngle(double x, double y)
        {
            while (y - 360.0 >= x)
                y -= 360.0;
            while (y < x)
                y += 360.0;
            return y;
        }

        static double MapAzimuthToLinear(double left_az, double right_az, double azimuth)
        {
            double mid_az = (left_az + right_az) / 2.0;
            double az_range = right_az - mid_az;

            double rel_az = azimuth - mid_az;

            double gain_r = 0.5 + 0.5 * Math.Tan(ToRadians(rel_az)) / Math.Tan(ToRadians(az_range));

            return Math.Atan2(gain_r, 1 - gain_r) * (2 / Math.PI);
        }

        static double ToRadians(double angle)
        {
            return (Math.PI / 180.0) * angle;
        }
    }


}
