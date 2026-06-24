using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ExcelConverter
{
    class Block
    {
        public int n = 0;
        public double x_sum = 0;
        public double y_sum = 0;
        public double xy_sum = 0;
        public double xx_sum = 0;
        public double y = -9999;

        //public void Clear()
        //{
        //    n = 0;
        //    x_sum = 0;
        //    y_sum = 0;
        //    xy_sum = 0;
        //    xx_sum = 0;
        //    y = -9999;
        //}

        public void Add(double x, double y)
        {
            n++;
            x_sum += x;
            y_sum += y;
            xy_sum += x * y;
            xx_sum += x * x;
            if(this.y == -9999)
                this.y = y;
        }

        public void CopyFrom(Block other)
        {
            n = other.n;
            x_sum = other.x_sum;
            y_sum = other.y_sum;
            xy_sum = other.xy_sum;
            xx_sum = other.xx_sum;
            y = other.y;
        }
    }

    internal class Program
    {
        static void Main(string[] args)
        {
            double[] values = { 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.0, 25.1, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1};

            List<double> interpolated = InterpolateValues(values);

            foreach (var value in interpolated)
            {
                Console.WriteLine(value);
            }
        }

        static List<double> InterpolateValues(double[] values)
        {
            Block lastBlock = new Block();
            Block currentBlock = new Block();
            List<double> ret = new List<double>();

            for (int i = 0; i < values.Length; i++)
            {
                if (i == 171)
                    ;

                if (values[i] != currentBlock.y && values[i] != lastBlock.y) //next block detected
                {
                    if (lastBlock.n > 0) //not the first block
                    {
                        double m = CalculateM(lastBlock, currentBlock);
                        double t = CalculateT(lastBlock, currentBlock, m);

                        for (int j = i - currentBlock.n; j < i; j++)
                        {
                            double interpolatedValue = m * j + t;
                            ret.Add(interpolatedValue);
                        }
                    }

                    lastBlock.CopyFrom(currentBlock);
                    currentBlock = new Block();
                }

                currentBlock.Add(i, values[i]);
            }

            return ret;
        }

        static double CalculateM(Block block1, Block block2)
        {
            int n_sum = block1.n + block2.n;
            double x_sum = block1.x_sum + block2.x_sum;
            double y_sum = block1.y_sum + block2.y_sum;
            double xy_sum = block1.xy_sum + block2.xy_sum;
            double xx_sum = block1.xx_sum + block2.xx_sum;

            return (n_sum * xy_sum - x_sum * y_sum) / (n_sum * xx_sum - x_sum * x_sum);
        }

        static double CalculateT(Block block1, Block block2, double m)
        {
            int n_sum = block1.n + block2.n;
            double x_sum = block1.x_sum + block2.x_sum;
            double y_sum = block1.y_sum + block2.y_sum;

            return (y_sum - m * x_sum) / n_sum;
        }
    }
}