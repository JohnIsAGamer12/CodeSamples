using System.Collections;
using UnityEngine;

public class TriggerSwingingAx : TrapTriggers
{
    [Header("----- Swinging Ax -----")]
    [SerializeField] float startDelay;
    [SerializeField] GameObject heirTrap;    
    new Rigidbody rigidbody;

    new private void Start()
    {
        base.Start();
        rigidbody = mainTrap.transform.GetComponent<Rigidbody>();
    }

    // Update is called once per frame
    void Update()
    {
        if (playerHasTriggeredTrap)
            StartCoroutine(StartSwinging(startDelay));
    }

    IEnumerator StartSwinging(float delay)
    {
        rigidbody.constraints = RigidbodyConstraints.None;
        yield return new WaitForSeconds(delay);
        // if the player is not dead
        // Destroy the Gameobject and replace it with the normal trap
        // if the player is dead
        // then if the player respawn this trigger trap will be replaced with the normal Swinging Ax trap
    }
}
