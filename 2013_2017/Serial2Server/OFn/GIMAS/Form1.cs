using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.IO;

namespace GIMAS
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            //button initial
            button1.Text = "Start";
            button1.BackColor = Color.Green;
            //thread inital
            Control.CheckForIllegalCrossThreadCalls = false;
            serialPort1.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);

            // Sets g to a graphics object representing the drawing surface of the
            // control or form g is a member of.

        }

        public string olddata, indata, newdata, foldpath = "C:\\Temp";// data storage path and data procesing buffers
        static string x = "0";
        static string y = "0";
        static int xp = 0;
        static int yp=0;
        Pen myPen = new Pen(Color.Red);
        Graphics g = null;


        private void panel1_Paint(object sender, PaintEventArgs e)
        {
            int xd, yd;
            if (Int32.TryParse(x, out xd))
            {

            }
            if (Int32.TryParse(y, out yd))
            {

            }
            xp = xp + xd;
            yp = yp + yd;
            myPen.Width = 5;
            g = panel1.CreateGraphics();
            Point[] points = {
            new Point(panel1.Width/2,panel1.Height/2),
            new Point(xp/10+panel1.Width/2,yp/10+panel1.Height/2)
            };
            g.DrawLines(myPen, points);
            Freq.Text = xp.ToString();
            Testtime.Text = yp.ToString();
        }


        public int samplefilemarker, sampeNo, spacemarker1, spacemarker2, datatype, testnumberI = 1;// file marker,sampling NO.,space places, sensor data type and test number


        private void button1_Click(object sender, EventArgs e)
        {

            int flag;//used to different mouse click on stop to close port
            newdata = "";
            olddata = "";
            spacemarker1 = 0;
            spacemarker2 = 0;
            samplefilemarker = 1;

            datatype = 1;//1p1,2p2,3t1,4t2,5g1,6g2,7g3,8a1,9a2,10a3
            sampeNo = 0;
            flag = 0;// fist click if port is there, flag=1, label=stop. then it will not close
                     // second click, flag will be 0 and label =stop, it will close
                     // if port is not there, flag=0, label=stop. then it will close and back to inital state


            if (button1.Text == "Start")
            {
                serialPort1.Parity = Parity.None;
                serialPort1.PortName = "COM7";
                serialPort1.BaudRate = 38400;
                serialPort1.Handshake = Handshake.None;
                serialPort1.RtsEnable = true;
                serialPort1.DtrEnable = true;

                try
                {
                    button1.Text = "Stop";
                    button1.BackColor = Color.Gold;
                    flag = 1;


                    //FolderBrowserDialog dialog = new FolderBrowserDialog();
                    //dialog.Description = "Please select data storage path";
                    //if (dialog.ShowDialog() == DialogResult.OK)
                    //{
                    //    foldpath = dialog.SelectedPath;//selec storage places
                    //}
                    //else
                    //{
                    //    MessageBox.Show("No file path is selected, file will be stored in C:\\Temp");
                    //    foldpath = "C:\\Temp";
                    //}

                    //string testnumberS;
                    //testnumberS = testnumberI.ToString();
                    //testnumberI++;//one click one increase
                    //FileStream fs = new FileStream(foldpath + "\\Test" + testnumberS+"time.txt", FileMode.Append);// experiment time file
                    //StreamWriter sw = new StreamWriter(fs, Encoding.Default);
                    //string time;
                    //time = DateTime.Now.ToString();
                    //sw.Write(time);
                    //sw.Flush();
                    //sw.Close();
                    //fs.Close();
                    //foldpath = foldpath + "\\Test" + testnumberS; //update the filename for data
                    serialPort1.Open();
                }
                catch (IOException ex)
                {
                    MessageBox.Show(ex.Message);
                    flag = 0;// if it can not be open, we should close it
                }

            }

            if (button1.Text == "Stop" && flag == 0)
            {
                button1.Text = "Start";
                button1.BackColor = Color.Green;
                serialPort1.Close();

            }
        }
        private void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            panel1.Refresh();
            SerialPort sp = (SerialPort)sender;
            olddata = newdata;
            indata = sp.ReadExisting();
            string sampleNos;
            int sampleNoforfile, sampleNoforchart;//
            sampleNoforfile = sampeNo / 8000 + 1;//that control the storage file size 144000/8000=18 files 144000<int_max
            sampleNoforchart = sampeNo / 1000 + 1;//that control when torefresh the graph and text
                                                  //sampleNos=sampleNoforfile.ToString();//every 1000samples, update the storage file
                                                  //FileStream fs = new FileStream(foldpath+"_"+sampleNos+".txt", FileMode.Append);
                                                  //StreamWriter sw = new StreamWriter(fs, Encoding.Default);
                                                  //sw.Write(indata);//just write coming data
                                                  //sw.Flush();
                                                  //sw.Close();
                                                  //fs.Close();
            newdata = olddata + indata;//add coming data to old data


            for (int i = 0; i < newdata.Length; i++)//update label and chart
            {
                if (String.Equals(Char.ToString(newdata[i]), " ") && i > spacemarker2)
                {
                    spacemarker1 = spacemarker2;
                    spacemarker2 = i;
                    if (datatype == 1)
                    {
                        x = newdata.Substring(spacemarker1, spacemarker2 - spacemarker1);

                    }
                    else if (datatype == 2)
                    {
                        y = newdata.Substring(spacemarker1, spacemarker2 - spacemarker1);

                        datatype = 0;

                    }
                    datatype++;//update sensor type
                    String testtime;
                    float frequency, testtimef;
                    frequency = float.Parse(Freq.Text);
                    testtimef = sampeNo / frequency / 60;
                    testtime = testtimef.ToString();
                    Testtime.Text = (testtime);

                }
            }

            textBox1.Text = newdata;       //update text box

            if (sampleNoforchart > samplefilemarker)//sampling enough for one sampling cycle, update chart
            {
                samplefilemarker = sampleNoforchart;//update marker
                newdata = "";//update buffer
                spacemarker2 = 0;//update spacemarker and sensor type
                spacemarker1 = 0;
                datatype = 1;
                chart1.Series["P1"].Points.Clear(); chart1.Series["P2"].Points.Clear();
                chart2.Series["T1"].Points.Clear(); chart2.Series["T2"].Points.Clear();
                chart3.Series["A1"].Points.Clear(); chart3.Series["A2"].Points.Clear(); chart3.Series["A3"].Points.Clear();
                chart4.Series["G1"].Points.Clear(); chart4.Series["G2"].Points.Clear(); chart4.Series["G3"].Points.Clear();
                chart1.Refresh();
                chart1.Update();
                chart2.Refresh();
                chart2.Update();
                chart3.Refresh();
                chart3.Update();
                chart4.Refresh();
                chart4.Update();



            }
            textBox1.SelectionStart = textBox1.Text.Length;
            textBox1.ScrollToCaret();
        }



    }

}
