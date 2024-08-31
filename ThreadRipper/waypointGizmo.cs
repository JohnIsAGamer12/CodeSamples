using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class waypointGizmo : MonoBehaviour
{
    [Range(1, 3)][SerializeField] int waypointChildRad;
    [Range(1, 10)][SerializeField] public int waypointSearchRad;

    public static waypointGizmo instance;

    public void Awake()
    {
        instance = this;
    }


    private void OnDrawGizmos()
    {
        Gizmos.color = Color.green;
        Gizmos.DrawWireSphere(transform.position, waypointSearchRad);

        foreach (Transform t in transform)
        {
            Gizmos.color = Color.blue;
            Gizmos.DrawWireSphere(t.position, waypointChildRad);
        }

        for (int i = 0; i < transform.childCount; i++) 
        {
            Transform t = transform.GetChild(i);
            if (i == transform.childCount - 1)
            {
                Debug.DrawLine(t.position, transform.GetChild(0).position, Color.red);
            }
            else
                Debug.DrawLine(t.position, transform.GetChild(i+1).position, Color.red);
        }
    }
}
