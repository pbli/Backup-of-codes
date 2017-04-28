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
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;

namespace GIMAS
{
    //define of the server class
    public class ServerClient
    {
        public TcpClient tcp;
        public string clientName;

        public ServerClient(TcpClient clientSocket)
        {
            clientName = "Guest";
            tcp = clientSocket;
        }
    }

    public partial class Form1 : Form
    {
        private List<ServerClient> clients;
        private List<ServerClient> disconncetList;
        private TcpListener server;
        public int port = 1632;
        private bool serverStarted;
        public string newstring, jsonstring;
        public int qcount;
        public string qw, qx, qy, qz;

        //construction funciton
        public Form1()
        {
            InitializeComponent();
            //button initial
            button1.Text = "Start";
            button1.BackColor = Color.Green;
            //thread inital
            Control.CheckForIllegalCrossThreadCalls = false;
            serialPort1.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);

            clients = new List<ServerClient>();// clients list
            disconncetList = new List<ServerClient>();//disctd list
            qw = qx = qy = qz = "0";

            try
            {
                server = new TcpListener(IPAddress.Any, port);//any coming is ok
                server.Start();
                StartListening();
                serverStarted = true;
                MessageBox.Show("server has been started at port" + port.ToString());
            }
            catch (System.Exception ee)
            {
                MessageBox.Show("Socket error" + ee.Message);
            }
        }



        private void Form1_Load(object sender, EventArgs e)
        {

        }


        private void button1_Click(object sender, EventArgs e)
        {
            int flag;//used to different mouse click on stop to close port
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
            SerialPort sp = (SerialPort)sender;

            newstring = sp.ReadLine();//new data
            if (newstring != "")
            {
                if (newstring == "q")
                    qcount = 0;
                else if (qcount == 0)//w
                {
                    qw = newstring;
                    qcount += 1;
                }

                else if (qcount == 1)//x
                {
                    qx = newstring;
                    qcount += 1;
                }
                else if (qcount == 2)//y
                {
                    qy = newstring;
                    qcount += 1;
                }
                else if (qcount == 3)//z
                {
                    qz = newstring;
                    qcount += 1;
                }
                //MessageBox.Show(q0 +" "+ q1 +" "+ q2 +" "+ q3);
                jsonstring = "{" + "\"qw\"" + ":" + qw + ","
                     + "\"qx\"" + ":" + qx + ","
                      + "\"qy\"" + ":" + qy + ","
                       + "\"qz\"" + ":" + qz + "}";
                //MessageBox.Show(jsonstring);
                textBox1.Text = jsonstring;

                if (!serverStarted)
                    return;
                if (clients.Count == 0)
                    return;
                else
                {
                    try
                    {
                        foreach (ServerClient c in clients)
                        {
                            //is the client still connected
                            if (!IsConnected(c.tcp))
                            {
                                c.tcp.Close();
                                disconncetList.Add(c);
                                clients.Remove(c);
                                MessageBox.Show("hololens is disconncted");
                                continue;
                            }
                            else
                            {
                                NetworkStream s = c.tcp.GetStream();
                                if (s.DataAvailable)//data from client
                                {
                                    StreamReader reader = new StreamReader(s, true);
                                    string data = reader.ReadLine();
                                    if (data != null)
                                        OnIncomingData(c, data);
                                }
                                BroadCast(jsonstring, clients);
                            }
                        }
                    }
                    catch (Exception ee)
                    {
                        MessageBox.Show(ee.Message);
                    }
                }

            }
        }

        //check connected?
        private bool IsConnected(TcpClient c)
        {
            try
            {
                if (c != null && c.Client != null && c.Client.Connected)
                {
                    if (c.Client.Poll(0, SelectMode.SelectRead))
                    {
                        return !(c.Client.Receive(new byte[1], SocketFlags.Peek) == 0);
                    }
                    return true;
                }
                else
                    return false;
            }
            catch
            {
                return false;
            }

        }


        //new client connected
        private void AcceptTcpClient(IAsyncResult ar)
        {
            TcpListener listener = (TcpListener)ar.AsyncState;
            clients.Add(new ServerClient(listener.EndAcceptTcpClient(ar)));
            StartListening();
            MessageBox.Show("Hololens is connected");
            // send a message to everyone, say someone connected
            // BroadCast(clients[clients.Count - 1].clientName + " has connected", clients);
        }

        //start server
        private void StartListening()
        {
            server.BeginAcceptTcpClient(AcceptTcpClient, server);
        }

        //get data from client
        private void OnIncomingData(ServerClient c, string data)
        {
            MessageBox.Show(c.clientName + data);
        }

        //broadcast to all
        private void BroadCast(string data, List<ServerClient> cl)
        {
            foreach (ServerClient c in cl)
            {
                try
                {
                   StreamWriter  writer = new StreamWriter(c.tcp.GetStream());
                    writer.WriteLine(data);
                    writer.Flush();
                }
                catch (Exception e)
                {
                    MessageBox.Show("writer error:" + e.Message + "to client" + c.clientName);
                }
            }
        }

    }

}
