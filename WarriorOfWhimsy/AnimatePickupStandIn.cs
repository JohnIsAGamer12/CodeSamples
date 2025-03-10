using System.Collections;
using UnityEngine;

public class AnimatePickupStandIn : MonoBehaviour
{
    [SerializeField] float lerpSpeed;

    // Update is called once per frame
    void FixedUpdate()
    {
        transform.Rotate(0, lerpSpeed, 0);
    }
}
