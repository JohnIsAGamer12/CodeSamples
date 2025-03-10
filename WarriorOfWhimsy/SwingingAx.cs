using UnityEngine;

public class SwingingAx : MonoBehaviour, ICombat
{
    [Header("----- ICombat Settings -----")]
    [SerializeField] BoxCollider hitBoxCollider;
    [SerializeField] int axDamage;

    [Header("----- Animation Settings -----")]
    [SerializeField] float swingingSpeed;
    [SerializeField] int maxAngle;
    [SerializeField] bool oppositeDir;

    void Update()
    {
        AnimateSwingingAx();
    }

    #region Animation

    void AnimateSwingingAx()
    {
        float angle;

        if (!oppositeDir)
            angle = maxAngle * Mathf.Sin(Time.time * swingingSpeed);
        else
            angle = -maxAngle * Mathf.Sin(Time.time * swingingSpeed);
        
        transform.localRotation = Quaternion.Euler(0, 0, angle);
    }

    #endregion

    #region Functions from Combat Interface

    public void DealHit(ICombat _reciever) 
    {
        _reciever.ReceiveHit(hitBoxCollider, axDamage);
    }

    public void ReceiveHit(Collider _dealer, float _dam) { }

    public bool GetIsAtking() { return false; }

    public bool GetIsBlocking() { return false; }

    #endregion

    #region BoxCollider Getter

    public BoxCollider GetBoxCollider() { return hitBoxCollider; }

    #endregion
}