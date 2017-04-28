using System;
using UnityEngine;
using System.Collections;
#if !UNITY_EDITOR
using Windows.Networking.Sockets;
using Windows.ApplicationModel.Core;
using Windows.Storage.Streams;
#endif
public class changeText : MonoBehaviour {

    public string Message { get; set; }

    // Use this for initialization
    void Start () {
        #if !UNITY_EDITOR
        StartListener();
#endif
        this.Message = "f";
    }

    // Update is called once per frame
    void Update () {
        GetComponent<TextMesh>().text = this.Message;
    }
#if !UNITY_EDITOR
    async void StartListener()
    {
        
        StreamSocketListener listener = new StreamSocketListener();
        listener.ConnectionReceived += OnConnection;

        // If necessary, tweak the listener's control options before carrying out the bind operation.
        // These options will be automatically applied to the connected StreamSockets resulting from
        // incoming connections (i.e., those passed as arguments to the ConnectionReceived event handler).
        // Refer to the StreamSocketListenerControl class' MSDN documentation for the full list of control options.
        listener.Control.KeepAlive = false;

        // Save the socket, so subsequent steps can use it.
        CoreApplication.Properties.Add("listener", listener);

        // Start listen operation.
        try
        {
            // Don't limit traffic to an address or an adapter.
            await listener.BindServiceNameAsync("22112");
        }
        catch (Exception exception)
        {
            CoreApplication.Properties.Remove("listener");

            // If this is an unknown status it means that the error is fatal and retry will likely fail.
            if (SocketError.GetStatus(exception.HResult) == SocketErrorStatus.Unknown)
            {
                throw;
            }
        }
    }

    private async void OnConnection(StreamSocketListener sender, StreamSocketListenerConnectionReceivedEventArgs args)
    {
        DataReader reader = new DataReader(args.Socket.InputStream);
        try
        {
            while (true)
            {
                // Read first 4 bytes (length of the subsequent string).
                uint sizeFieldCount = await reader.LoadAsync(sizeof(uint));
                if (sizeFieldCount != sizeof(uint))
                {
                    // The underlying socket was closed before we were able to read the whole data.
                    return;
                }

                // Read the string.
                uint stringLength = reader.ReadUInt32();
                uint actualStringLength = await reader.LoadAsync(stringLength);
                if (stringLength != actualStringLength)
                {
                    // The underlying socket was closed before we were able to read the whole data.
                    return;
                }
                this.Message = reader.ReadString(actualStringLength);
            }
        }
        catch (Exception exception)
        {
            // If this is an unknown status it means that the error is fatal and retry will likely fail.
            if (SocketError.GetStatus(exception.HResult) == SocketErrorStatus.Unknown)
            {
                throw;
            }
        }
    }

#endif
}
