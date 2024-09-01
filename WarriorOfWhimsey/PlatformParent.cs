using UnityEngine;

public class PlatformParent : MonoBehaviour
{
    private void OnTriggerEnter(Collider other)
    {
        if (other.isTrigger)
            return;

        if (other.CompareTag("Player"))
        {
            other.transform.SetParent(transform);
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.isTrigger)
            return;

        if (other.CompareTag("Player"))
        {
            other.transform.SetParent(null);
        }
    }
}
