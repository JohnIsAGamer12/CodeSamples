using UnityEngine;

public class MovingRocks : MonoBehaviour
{
    [SerializeField] GameObject _platform;
    [SerializeField] GameObject _pivot;
    [SerializeField] float swayingSpeed;
    [SerializeField] int maxVerticalAngle;
    [SerializeField] int maxHorizontalAngle;
    [SerializeField] bool oppositeDir;

    private void Start()
    {
        _pivot = transform.GetChild(0).gameObject;
        _platform = _pivot.transform.GetChild(0).gameObject;
    }

    // Update is called once per frame
    void FixedUpdate()
    {
        AnimateMovingRocks();
    }

    #region Animation

    void AnimateMovingRocks()
    {
        float verticalAngle;
        float horizontalAngle;

        if (!oppositeDir)
        {
            verticalAngle = maxVerticalAngle * Mathf.Sin(Time.time * swayingSpeed);
            horizontalAngle = maxHorizontalAngle * Mathf.Sin(Time.time * swayingSpeed);
        }
        else
        {
            verticalAngle = -maxVerticalAngle * Mathf.Sin(Time.time * swayingSpeed);
            horizontalAngle = -maxHorizontalAngle * Mathf.Sin(Time.time * swayingSpeed);
        }

        _pivot.transform.localRotation = Quaternion.Euler(0, horizontalAngle, verticalAngle);
        _platform.transform.localRotation = Quaternion.Euler(0, -horizontalAngle, -verticalAngle);
    }

    #endregion
}
