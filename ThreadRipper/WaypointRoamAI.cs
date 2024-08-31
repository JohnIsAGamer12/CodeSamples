using System.Collections;
using UnityEngine;

public class WaypointRoamAI : BaseAI
{
    [Header("----- Waypoint Roam Requirements -----")]
    [SerializeField] Transform waypoint;
    [SerializeField] int roamPause;

    public bool walkpointSet; // public for debug reasons

    Coroutine waypointCot;
    int currentWaypointIndex;
    Transform currentWaypoint;

    // Start is called before the first frame update
    void Start()
    {
        EnemyManager.instance.enemies.Add(gameObject);
        waypoint = FindNearestWaypoint();
        if (waypoint == null)
            Debug.LogError("Hey! " + transform.name + "Couldn't find a Waypoint near him! Put him closer to one!");

        walkpointSet = false;
        currentWaypointIndex = 0;
        transform.position = waypoint.GetChild(0).position;
    }

    public override void patrol()
    {
        if (!walkpointSet)
        {
            agent.stoppingDistance = 0;
            waypointCot = StartCoroutine(GetNearestWaypoint(roamPause));
        }
    }

    public override bool canSeePlayer()
    {
        if (base.canSeePlayer())
        {
            if (playerdetected)
            {
                return true;
            }
            else if (!playerdetected)
            {
                if (waypointCot != null)
                    StopCoroutine(waypointCot);
                walkpointSet = true;
            }
        }

        walkpointSet = false;
        return false;
    }

    public Transform FindNearestWaypoint()
    {
        for (int i = 0; i < EnemyManager.instance.waypoints.Length; ++i)
        {
            if (Vector3.Distance(transform.position, EnemyManager.instance.waypoints[i].transform.position) <= waypointGizmo.instance.waypointSearchRad)
            {
                Debug.Log("Waypoint assigned to " + transform.name);
                return EnemyManager.instance.waypoints[i].transform;
            }
        }

        return null;
    }

    public IEnumerator GetNearestWaypoint(int delay)
    {
        if (agent.isOnNavMesh)
        {
            if (agent.remainingDistance < 0.01f)
            {
                walkpointSet = true;
                agent.stoppingDistance = 0;
                yield return new WaitForSeconds(delay);
                if (currentWaypointIndex <= waypoint.childCount - 1)
                {
                    currentWaypoint = waypoint.GetChild(currentWaypointIndex);
                    currentWaypointIndex++;
                }
                else
                {
                    currentWaypoint = waypoint.GetChild(0);
                    currentWaypointIndex = 0;
                }
                agent.SetDestination(currentWaypoint.position);
                walkpointSet = false;
            }
        }
    }

    public override void TakeDamage(float damage)
    {
        walkpointSet = false;
        if (waypointCot != null)
            StopCoroutine(waypointCot);
        base.TakeDamage(damage);
    }
}