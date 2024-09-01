using UnityEngine;

public class AxHitBox : MonoBehaviour
{
    BoxCollider wielder;
    ICombat dealingCombatant;

    private void Awake()
    {
        wielder = GetComponent<BoxCollider>();

        if (!wielder.TryGetComponent(out dealingCombatant))
        {
            Debug.LogError("Please us the ICombat interface for the Ax!");
            Destroy(gameObject);
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.isTrigger)
            return;

        if (!other.TryGetComponent(out ICombat hitCombatant))
        {
            // Not a Combatant
            return;
        }

        if (hitCombatant == dealingCombatant)
        {
            // Hit self
            return;
        }

        dealingCombatant.DealHit(hitCombatant);
    }
}
