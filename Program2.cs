using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TempKoeff
{
    using uint8_t = Byte;
    using int16_t = Int16;
    using uint16_t = UInt16;
    using int8_t = SByte;

    class temperature_prediction_rolling_buffer_t
    {
        public float[] data;             // the data points
        public int head_index;          // index of the most recent entry (-1 when empty)
        public int length;              // current number of valid entries in the buffer
        public int max;                 // maximum capacity of the buffer
    }

    class Program
    {
        static int MAX = 60;
        static int idx1 = -1;
        static int idx2 = -1;

        static void Main(string[] args)
        {
            //List<float> allData = new List<float>()  { 25.9F, 25.9F, 25.9F, 25.9F, 25.9F, 25.9F, 25.9F, 25.9F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.8F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.7F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.6F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.5F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.4F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F, 25.3F };
            List<float> allData = new List<float>() { 9F, 9F, 9F, 9F, 9F, 9F, 9F, 9F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 8F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 7F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 6F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 5F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 4F, 3F, 3F, 3F, 3F, 3F, 3F, 3F, 3F, 3F, 3F, 3F, 3F, 3F };
            //allData = new List<float>() { 14.2F, 14.2F, 14.2F, 14.2F, 14.2F, 14.3F, 14.3F, 14.3F, 14.3F, 14.3F, 14.3F, 14.4F, 14.4F, 14.4F, 14.4F, 14.4F, 14.5F, 14.5F, 14.5F, 14.5F, 14.5F, 14.5F, 14.6F, 14.6F, 14.6F, 14.6F, 14.6F, 14.7F, 14.7F, 14.7F, 14.7F, 14.7F, 14.7F, 14.8F, 14.8F, 14.8F, 14.8F, 14.8F, 14.8F, 14.9F, 14.9F, 14.9F, 14.9F, 14.9F, 14.9F, 15F, 15F, 15F, 15F, 15F, 15F, 15F, 15F, 15.1F, 15.1F, 15.1F, 15.1F, 15.1F, 15.1F, 15.2F, 15.2F, 15.2F, 15.2F, 15.2F, 15.2F, 15.2F, 15.3F, 15.3F, 15.3F, 15.3F, 15.3F, 15.3F, 15.3F, 15.4F, 15.4F, 15.4F, 15.4F, 15.4F, 15.5F, 15.5F, 15.5F, 15.5F, 15.5F, 15.5F, 15.5F, 15.5F, 15.6F, 15.6F, 15.6F, 15.6F, 15.6F, 15.6F, 15.6F, 15.7F, 15.7F, 15.7F, 15.7F, 15.7F, 15.7F, 15.7F, 15.7F, 15.8F, 15.8F, 15.8F, 15.8F, 15.8F, 15.8F, 15.8F, 15.8F, 15.8F, 15.9F, 15.9F, 15.9F, 15.9F, 15.9F, 15.9F, 15.9F, 15.9F, 16F, 16F, 16F, 16F, 16F, 16F, 16F, 16F, 16F, 16.1F, 16.1F, 16.1F, 16.1F, 16.1F, 16.1F, 16.1F, 16.1F, 16.1F, 16.2F, 16.2F, 16.2F, 16.2F, 16.2F, 16.2F, 16.2F, 16.2F, 16.3F, 16.3F, 16.3F, 16.3F, 16.3F, 16.3F, 16.3F, 16.3F, 16.3F, 16.4F, 16.4F, 16.4F, 16.4F, 16.4F, 16.4F, 16.4F, 16.4F, 16.4F, 16.4F, 16.5F, 16.5F, 16.5F, 16.5F, 16.5F, 16.5F, 16.5F, 16.5F, 16.5F, 16.5F, 16.5F, 16.6F, 16.6F, 16.6F, 16.6F, 16.6F, 16.6F, 16.6F, 16.6F, 16.6F, 16.6F, 16.7F, 16.7F, 16.7F, 16.7F, 16.7F, 16.7F, 16.7F, 16.7F, 16.7F, 16.7F, 16.7F, 16.7F, 16.8F, 16.8F, 16.8F, 16.8F, 16.8F, 16.8F, 16.8F, 16.8F, 16.8F, 16.8F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 16.9F, 17F, 17F, 17F, 17F, 17F, 17F, 17F, 17F, 17F, 17F, 17F, 17F, 17.1F, 17.1F, 17.1F, 17.1F, 17.1F, 17.1F, 17.1F, 17.1F, 17.1F, 17.1F, 17.1F, 17.2F, 17.2F, 17.2F, 17.2F, 17.2F, 17.2F, 17.2F, 17.2F, 17.2F, 17.2F, 17.2F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.3F, 17.4F, 17.4F, 17.4F, 17.4F, 17.4F, 17.4F, 17.4F, 17.4F, 17.4F, 17.4F, 17.4F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.5F, 17.6F, 17.6F, 17.6F, 17.6F, 17.6F, 17.6F, 17.6F, 17.6F, 17.6F, 17.6F, 17.6F, 17.6F, 17.7F, 17.7F, 17.7F, 17.7F, 17.7F, 17.7F, 17.7F, 17.7F, 17.7F, 17.7F, 17.7F, 17.8F, 17.8F, 17.8F, 17.8F, 17.8F, 17.8F, 17.8F, 17.8F, 17.8F, 17.8F, 17.8F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 17.9F, 18F, 18F, 18F, 18F, 18F, 18F, 18F, 18F, 18F, 18F, 18F, 18F, 18F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.1F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.2F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.3F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.4F, 18.5F, 18.5F, 18.5F, 18.5F, 18.5F, 18.5F, 18.5F, 18.5F, 18.5F, 18.5F, 18.5F, 18.5F, 18.6F, 18.6F, 18.6F, 18.6F, 18.6F, 18.6F, 18.6F, 18.6F, 18.6F, 18.6F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.7F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.8F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 18.9F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.1F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.2F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.3F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.4F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.5F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.6F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.7F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.8F, 19.9F, 19.8F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 19.9F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.1F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.2F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.3F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.4F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.5F, 20.6F, 20.5F, 20.6F, 20.5F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.6F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.7F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.8F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 20.9F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21F, 21.1F, 21.1F, 21F };

            temperature_prediction_rolling_buffer_t buffer = new temperature_prediction_rolling_buffer_t();
            buffer.data = new float[MAX];
            buffer.head_index = -1;
            buffer.length = 0;
            buffer.max = MAX;

            for (int i = 0; i < allData.Count; i++)
            {
                if (i == 82)
                    ;

                temperature_prediction_face_add_to_rolling_buffer(buffer, allData[i]);

                idx1 = -1;
                idx2 = -1;
                var temp = temperature_prediction_face_calculate_end_temperature2(buffer, 1, 0.002F);

                int idx = 0;
                foreach (var f in buffer.data)
                {
                    if (idx == idx1 || idx == idx2)
                        Console.Write(">");
                    Console.Write($"{f}");
                    if (idx == idx1 || idx == idx2)
                        Console.Write("<");
                    Console.Write(" ");
                    idx++;
                }
                Console.WriteLine();
                
                Console.WriteLine($"tick {i}: {temp}");
                Console.WriteLine();
            }
        }

        static void temperature_prediction_face_add_to_rolling_buffer(temperature_prediction_rolling_buffer_t buffer, float value)
        {
            buffer.head_index = (buffer.head_index + 1) % buffer.max;
            buffer.length = buffer.length + 1 < buffer.max ? buffer.length + 1 : buffer.max;
            buffer.data[buffer.head_index] = value;
        }


        static void Umwandeln()
        {
            int[] seconds = { 0, 1, 3, 5, 7, 9, 10, 12, 13, 15, 17, 19, 21, 23, 26, 28, 30, 32, 34, 37, 39, 42, 44, 47, 49, 52, 55, 58, 61, 64, 67, 70, 73, 77, 80, 84, 88, 91, 95, 99, 102, 106, 110, 114, 119, 123, 128, 132, 137, 142, 147, 153, 158, 164, 169, 175, 180, 186, 192, 198, 206, 212, 219, 226, 231, 239, 246, 254, 263, 271, 280, 289, 297, 306, 316, 327, 337, 349, 359, 372, 384, 395, 406, 419, 430, 443, 455, 466, 477, 490, 503, 517, 532, 546, 560, 572, 582, 598, 613, 628, 644, 660, 677, 693, 708, 728, 748, 770, 790, 791, 792, 814, 835, 857, 878, 901, 924, 950, 951, 952, 953, 954, 982, 1012, 1039, 1075, 1104, 1106 };
            double[] temp = { 9.1, 9.2, 9.3, 9.4, 9.5, 9.6, 9.7, 9.8, 9.9, 10, 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8, 10.9, 11, 11.1, 11.2, 11.3, 11.4, 11.5, 11.6, 11.7, 11.8, 11.9, 12, 12.1, 12.2, 12.3, 12.4, 12.5, 12.6, 12.7, 12.8, 12.9, 13, 13.1, 13.2, 13.3, 13.4, 13.5, 13.6, 13.7, 13.8, 13.9, 14, 14.1, 14.2, 14.3, 14.4, 14.5, 14.6, 14.7, 14.8, 14.9, 15, 15.1, 15.2, 15.3, 15.4, 15.5, 15.6, 15.7, 15.8, 15.9, 16, 16.1, 16.2, 16.3, 16.4, 16.5, 16.6, 16.7, 16.8, 16.9, 17, 17.1, 17.2, 17.3, 17.4, 17.5, 17.6, 17.7, 17.8, 17.9, 18, 18.1, 18.2, 18.3, 18.4, 18.5, 18.6, 18.7, 18.8, 18.9, 19, 19.1, 19.2, 19.3, 19.4, 19.5, 19.6, 19.7, 19.8, 19.9, 19.8, 19.9, 20, 20.1, 20.2, 20.3, 20.4, 20.5, 20.6, 20.5, 20.6, 20.5, 20.6, 20.7, 20.8, 20.9, 21, 21.1, 21 };

            StringBuilder sbs = new StringBuilder();
            StringBuilder sbt = new StringBuilder();
            int idx = 0;
            for (int sec = 0; sec <= seconds[seconds.Length - 1]; sec++)
            {
                if (idx + 1 < seconds.Length && seconds[idx + 1] == sec)
                    idx++;

                //if (sec % 60 == 0)
                {
                    sbs.AppendLine(sec.ToString());
                    sbt.AppendLine(temp[idx].ToString());
                }
            }

            System.IO.File.WriteAllText("sbs.txt", sbs.ToString());
            System.IO.File.WriteAllText("sbt.txt", sbt.ToString());
        }

        static float temperature_prediction_face_calculate_end_temperature2(temperature_prediction_rolling_buffer_t buffer, uint8_t block_gap, float coefficient)
        {
            int data_index = buffer.head_index;
            int8_t block_index = -1;
            uint16_t block_size = 0; //current block size
            float last_block_temperature = -999;
            float current_block_temperature = -999;
            int middle_index_temperature1 = -999;
            float temperature1 = -999; //middle temperature of newest complete block
            float temperature2 = -999; //middle temperature of oldest complete block that gets taken into account
            int delta = 1; //delta in seconds between the two temperatures

            // Iterate backwards from newest entry to oldest, to find blocks
            // Each block is a group of consecutive same temperatures (with a tolerance -> 1111212222 is counted as two blocks, 1111 and 212222, because sensor sometimes does this)   
            for (uint16_t cnt = 0; cnt < buffer.length && block_index < block_gap + 2; cnt++)
            {
                if (buffer.data[data_index] != current_block_temperature && buffer.data[data_index] != last_block_temperature)
                { //next block detected: block block_index complete
                    if (block_index > 0)
                    { //after first complete block
                        int middle_index = (data_index + ((block_size + 1) / 2)) % buffer.length; //index of the middle element of the block
                        if (block_index == 1)
                        {
                            temperature1 = current_block_temperature;
                            middle_index_temperature1 = middle_index;
                        }
                        else
                        {
                            temperature2 = current_block_temperature;
                            delta = ((int)middle_index_temperature1 - (int)middle_index + buffer.length) % buffer.length;

                            Console.WriteLine($"index 1: {middle_index_temperature1}");
                            Console.WriteLine($"index 2: {middle_index}");
                            Console.WriteLine($"delta: {delta}");
                            idx1 = middle_index_temperature1;
                            idx2 = middle_index;
                        }
                    }
                    block_index++;
                    block_size = 0;
                    last_block_temperature = current_block_temperature;
                    current_block_temperature = buffer.data[data_index];
                }
                block_size++;
                data_index = data_index == 0 ? buffer.length - 1 : data_index - 1; //move index backwards with wrap around
            }



             if (block_index < block_gap + 2) { //TODO: if buffer is not big enough, we might find 3 blocks but the oldest one might not be complete. So we can search for 4 blocks, to be sure. But at the beginning of the measurement, we only have 3 blocks. 
                 return 999; //not enough blocks found
             }

            //TODO: buffer size must be set to MAX when using algorithm2

            return temperature_correction_face_calculate_end_temperature_raw(delta, temperature1, temperature2, coefficient);
        }

        static float temperature_correction_face_calculate_end_temperature_raw(int delta, float temperature_current, float temperature_start, float coefficient)
        {
            //=(Tcurrent-Tstart*EXP(-k*time))/(1-EXP(-k*time))
            float ex = expf(-coefficient * (float)delta);
            return (temperature_current - temperature_start * ex) / (1 - ex);
        }

        static float expf(float val)
        {
            return (float)Math.Exp(val);
        }
    }
}
