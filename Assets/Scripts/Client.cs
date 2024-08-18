using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using UnityEngine;

public class Client : MonoBehaviour
{
    public ushort UserId
    {
        get;
        set;
    }

    private Socket socket;
    private Thread receiveThread;
    private Queue<string> receiveQueue = new Queue<string>();
    public Queue<string> ReceiveQueue => receiveQueue;

    private void OnApplicationQuit()
    {
        if (socket != null && socket.Connected)
        {
            Disconnect();
        }
    }

    private void ReceiveThread()
    {
        while (true)
        {
            byte[] data = new byte[1];

            socket.Receive(data, 1, SocketFlags.None);
            receiveQueue.Enqueue(Encoding.ASCII.GetString(data, 0, 1));
        }
    }

    public void ConnectToServer()
    {
        socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

        socket.Connect(IPAddress.Parse("127.0.0.1"), 2714);

        receiveThread = new Thread(ReceiveThread);

        receiveThread.Start();

        Send(Command.CMD_C2S_JOIN_GAME, string.Empty);
    }

    public void Send(uint command, string message)
    {
        byte[] data = Encoding.ASCII.GetBytes($"{command}|{message}");

        socket.Send(data, data.Length, SocketFlags.None);
    }

    public uint GetCommandByData(string data)
    {
        if (data == null || data.Equals(string.Empty))
        {
            return Command.INVALID_CMD;
        }

        string[] split = data.Split('|');

        return split.Length > 0 && uint.TryParse(split[0], out uint result) ? result : Command.INVALID_CMD;
    }

    public void Disconnect()
    {
        Send(Command.CMD_C2S_LEAVE_GAME, string.Empty);

        if (receiveThread != null)
        {
            receiveThread.Abort();
        }

        receiveQueue.Clear();
        socket.Close();

        socket = null;
    }
}
