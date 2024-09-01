using System.Collections;
using UnityEngine;
using UnityEngine.InputSystem;

public class BasePickUp : MonoBehaviour, IPickUp
{
    [Header("----- Base Components -----")]
    [SerializeField] GameObject model;
    public bool playerInRange;

    [Header("----- UI Settings -----")]
    [SerializeField] float displayHeight;

    [Header("----- Audio Object -----")]
    public GameObject audioPickup;
    [SerializeField] AudioClip pickUpClip;
    [Range(0f, 1f)][SerializeField] float pickUpSoundVol;
    [SerializeField] float audioCoolDown;

    [Header("----- Ambush Components (Optional) -----")]
    public AmbushTriggers ambush;
    [SerializeField] bool isAnAmbush;


    GameObject audioGameObj;


    private void Start()
    {
        InputManager.instance.playerInput.Player.Interact.started += OnInteract;
        if (isAnAmbush)
        {
            ambush = GetComponent<AmbushTriggers>();
            ambush.isAPickUp = true;
        }
    }
    private void OnDestroy()
    {
        InputManager.instance.playerInput.Player.Interact.canceled -= OnInteract;
        playerInRange = false;
    }

    public virtual void OnInteract(InputAction.CallbackContext context)
    {
        if (context.started && playerInRange)
        {
            HUDManager.instance.HideInteractPopup();
            PickUpSound audio = audioGameObj.GetComponent<PickUpSound>();

            if (audio != null)
                audio.PlaySound(pickUpClip, pickUpSoundVol, audioCoolDown);

            if (ambush != null)
            {
                if (ambush._enemies.Length > 0)
                    ambush.SpawnEnemies();
                else
                    Debug.LogError("You don't have any enemies in this list!");
            }
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            playerInRange = true;
            audioGameObj = Instantiate(audioPickup, transform.position, transform.rotation);
            HUDManager.instance.ShowInteractPopup(model.transform, displayHeight);
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            playerInRange = false;
            Destroy(audioGameObj);
            HUDManager.instance.HideInteractPopup();
        }
    }

    #region Virtual Functions from IPickup

    public virtual void GiveHealth() { }
    public virtual void GiveHeartShard() { }
    public virtual void GiveWeapon() { }

    #endregion
}
