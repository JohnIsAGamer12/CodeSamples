using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TrapTriggers : MonoBehaviour
{
    [Header("----- Trap Triggers Settings -----")]
    public GameObject mainTrap;
    public Collider trapTrigger;
    public bool playerHasTriggeredTrap;
    protected bool isAttacking;

    public void Start()
    {
        mainTrap = transform.GetChild(0).gameObject;
        trapTrigger = transform.GetComponent<Collider>();
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
            playerHasTriggeredTrap = true;
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.CompareTag("Player"))
            playerHasTriggeredTrap = false;
    }
}
