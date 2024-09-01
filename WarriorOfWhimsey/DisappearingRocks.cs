using System.Collections;
using Unity.VisualScripting;
using UnityEngine;

public class DisappearingRocks : TrapTriggers
{
    [Header("----- Disappearing Rocks Settings -----")]
    [SerializeField] float disappearingDelay;
    [SerializeField] float reappearDelay;
    public Collider boxCollider;

    [Header("----- Particle Settings -----")]
    [SerializeField] ParticleSystem unstableFX;
    [SerializeField] ParticleSystem poofFX;

    [Header("----- Audio Settings -----")]
    [SerializeField] AudioSource audioSource;
    [SerializeField] AudioClip[] crumblingClips;
    [Range(0f, 1f)][SerializeField] float crumbleSoundVol;

    GameObject groundDetector;
    GroundDetection groundScript;

    Coroutine disappear;
    Coroutine reappearing;

    bool hasDisappeared;

    // Start is called before the first frame update
    new private void Start()
    {
        base.Start();
        boxCollider = mainTrap.transform.GetComponent<Collider>();
        groundDetector = GameObject.Find("GroundDetector");
        groundScript = groundDetector.GetComponent<GroundDetection>();
    }

    // Update is called once per frame
    void Update()
    {
        if (playerHasTriggeredTrap && !hasDisappeared)
        {
            disappear = StartCoroutine(DisappearPlatform(disappearingDelay));
            playerHasTriggeredTrap = false;
        }
    }

    IEnumerator DisappearPlatform(float delay)
    {
        // Play sound
        audioSource.PlayOneShot(crumblingClips[Random.Range(0, crumblingClips.Length - 1)], crumbleSoundVol);
        yield return new WaitForSeconds(delay);
        poofFX.Play();
        groundScript.DeleteCurrGroundCol(boxCollider);
        boxCollider.enabled = false;
        mainTrap.GetComponent<MeshRenderer>().enabled = false;
        unstableFX.Stop();
        hasDisappeared = true;
        reappearing = StartCoroutine(ReappearPlatform(reappearDelay));
    }

    IEnumerator ReappearPlatform(float delay)
    {
        if (disappear != null)
        {
            StopCoroutine(disappear);
            disappear = null;
        }
        yield return new WaitForSeconds(delay);
        poofFX.Play();
        boxCollider.enabled = true;
        mainTrap.GetComponent<MeshRenderer>().enabled = true;
        unstableFX.Play();
        hasDisappeared = false;
    }
}
