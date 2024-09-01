using UnityEngine;
using UnityEngine.InputSystem;

public class HealthPickUp : BasePickUp
{
    [Header("----- Health PickUp Settings -----")]
    [SerializeField] int healAmount;
    IHealing playerHeal;

    public override void OnInteract(InputAction.CallbackContext context)
    {
        base.OnInteract(context);
        if (context.started && playerInRange)
        {
            GiveHealth();
            Destroy(gameObject);
        }
    }

    public override void GiveHealth()
    {
        playerHeal = PlayerController.Instance.GetComponent<IHealing>();

        if (playerHeal != null)
            playerHeal.HealPlayer(healAmount);
    }
}
