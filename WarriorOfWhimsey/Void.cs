using UnityEngine;

public class Void : MonoBehaviour
{
    Vector3 spawnDirection;
    GameObject[] checkPoints;

    private void Start()
    {
        checkPoints = CheckPointManager.instance.checkPoints;
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            PlayerController.Instance.ReceiveHit(other, 25);

            GameManager.instance.player.transform.position = CheckPointManager.instance.GetActiveCheckPoint().transform.position;
        }
    }
}
