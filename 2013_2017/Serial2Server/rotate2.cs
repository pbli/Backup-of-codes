using UnityEngine;
using System.Collections;
using System.IO;
using UnityEngine.UI;
using System.Collections.Generic;
using System;

#if UNITY_EDITOR
using System.Net;
using System.Net.Sockets;
#endif

#if !UNITY_EDITOR
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using Windows.Networking;
using Windows.Foundation;
#endif

[System.Serializable]
public class Par //seems unity json still not support array format
{
    public string P;//part no
    public float R;//rotation
    public float M;//linear motion
}
[System.Serializable]
public class ParCollection //seems unity json still not support array format
{
    public Par[] Pars;
}

public class rotate2 : MonoBehaviour
{

#if !UNITY_EDITOR
    static private  StreamSocket networkConnection;
    private string ServerIP = "129.93.15.115";//ip
    private int ConnectionPort = 1632;
    private float rot = 1.0f;
    private bool connectedh;

    public void Start()
    {
        connectedh = false;
    }

    public void Update()
    {
        if (!connectedh)//if hololens not connected
        {
            HostName networkHost = new HostName(ServerIP.Trim());
            networkConnection = new StreamSocket();

            // Connections are asynchronous.  
            // !!! NOTE These do not arrive on the main Unity Thread. Most Unity operations will throw in the callback !!!
            IAsyncAction outstandingAction = networkConnection.ConnectAsync(networkHost, ConnectionPort.ToString());
            AsyncActionCompletedHandler aach = new AsyncActionCompletedHandler(NetworkConnectedHandler);
            outstandingAction.Completed = aach;
        }
        else
        {
           readdata();
        }
            

    }
      public void readdata()
    {
        DataReader inputStream = new DataReader(networkConnection.InputStream);
        string response = string.Empty;
        inputStream.InputStreamOptions = InputStreamOptions.Partial;
        IAsyncOperation<uint> taskLoad = inputStream.LoadAsync(256);
        taskLoad.AsTask().Wait();
        uint content = taskLoad.GetResults();
        uint byteCount = inputStream.UnconsumedBufferLength;
        response = inputStream.ReadString(byteCount);
        GameObject newG = GameObject.Find("input1");
        newG.GetComponentInChildren<Text>().text = response;
    }
    void ReaderHandler(IAsyncAction asyncInfo, AsyncStatus status)
    {
        
    }
    void onDestroy()
    {
        networkConnection.Dispose();
    }

    void NetworkConnectedHandler(IAsyncAction asyncInfo, AsyncStatus status)
    {
        // Status completed is successful.
        if (status == AsyncStatus.Completed)
        {
            connectedh = true;
        }
        else
        {
            Debug.Log("Failed to establish connection. Error Code: " + asyncInfo.ErrorCode);
            networkConnection.Dispose();
        }
    }

#endif

#if UNITY_EDITOR  //for editor
    //internet connection 
    private bool socketReady;
    private TcpClient socket;
    private NetworkStream stream;
    private StreamReader reader;
    //private StreamWriter writer;
    private bool connected;

    private string jsstring;
    public ParCollection controlPars;
    public float rot;





    void Start()
    {
        rot = 1.0F;
        socketReady = false;
        connected = false;
    }

    // Update is called once per frame
    void Update()
    {
        if (!connected)//connect to server first time
            ConnectToServer();
        if (socketReady)//connected
        {
            if (stream.DataAvailable)
            {
                string data = reader.ReadLine();
                if (data != null)
                    OnIncomingData(data);
            }
        }


    }

    private void OnIncomingData(string data)
    {
        //string jsstring = File.ReadAllText(Application.dataPath + "/strings.json");
        //////in debug, need copy jsonfile to data under app/data every build will erase this folder
        //Debug.Log(jsstring);
        //Debug.Log(data);

        controlPars = JsonUtility.FromJson<ParCollection>(data);

        if (1 == 1)
        {
          //  gameObject.transform.GetChild(0).transform.Rotate(Vector3.left, controlPars.Pars[0].R);

          // gameObject.transform.GetChild(1).transform.Rotate(Vector3.left, controlPars.Pars[1].R);

        }
        if (Input.GetKey(KeyCode.DownArrow))
        {
            gameObject.transform.GetChild(0).transform.Rotate(Vector3.left, -rot);

            gameObject.transform.GetChild(1).transform.Rotate(Vector3.left, -rot);

        }
        if (1 == 1)
        {

            gameObject.transform.GetChild(0).transform.GetChild(2).transform.Rotate(Vector3.up, rot);
            gameObject.transform.GetChild(1).transform.GetChild(2).transform.Rotate(Vector3.up, -rot);
        }
        if (Input.GetKey(KeyCode.RightArrow))
        {

            gameObject.transform.GetChild(0).transform.GetChild(2).transform.Rotate(Vector3.up, -rot);
            gameObject.transform.GetChild(1).transform.GetChild(2).transform.Rotate(Vector3.up, rot);
        }

        gameObject.transform.GetChild(0).transform.GetChild(2).transform.GetChild(1).transform.Rotate(Vector3.back, rot * 5);


    }

    private void ConnectToServer()
    {
        if (socketReady)
        {
            return;
        }
        string host = "127.0.0.1";//ip
        int port = 1632;//port
        try
        {
            socket = new TcpClient(host, port);
            stream = socket.GetStream();
            //writer = new StreamWriter(stream);
            reader = new StreamReader(stream);
            socketReady = true;
            connected = true;
        }
        catch (Exception e)
        {
            Debug.Log("socket error" + e.Message);
        }


    }


#endif

}
