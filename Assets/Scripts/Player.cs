using System.Collections;
using UnityEngine;

public class Player : MonoBehaviour
{
    [SerializeField]
    private Client client;

    private IEnumerator Start()
    {
        client.ConnectToServer();

        while (true)
        {
            Move();

            if (client.ReceiveQueue.Count > 0)
            {
                string receiveData = client.ReceiveQueue.Dequeue();

                switch (client.GetCommandByData(receiveData))
                {
                    case Command.CMD_S2C_USER_ID:
                        client.UserId = ushort.Parse(receiveData);

                        break;
                }
            }

            yield return null;
        }
    }

    private void Move()
    {
        float horizontal = Input.GetAxisRaw("Horizontal");
        float vertical = Input.GetAxisRaw("Vertical");

        transform.position += (Vector3.right * horizontal + Vector3.up * vertical).normalized * Time.deltaTime;

        client.Send(Command.CMD_C2S_PLAYER_MOVEMENT, $"{client.UserId}|{transform.position.x},{transform.position.y}");
    }
}
