using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PickUpSound : MonoBehaviour
{
    [Header("----- Audio Components -----")]
    public AudioSource audioSource;

    public void PlaySound(AudioClip audioClip, float soundVol, float delay)
    {
        StartCoroutine(PlayPickUpSound(audioClip, soundVol, delay));
    }

    IEnumerator PlayPickUpSound(AudioClip audioClip, float soundVol, float delay)
    {
        audioSource.PlayOneShot(audioClip, soundVol);
        yield return new WaitForSeconds(audioClip.length);
        Destroy(gameObject);
    }
}
