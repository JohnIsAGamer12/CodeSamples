using System.Collections.Generic;
using UnityEngine;

public class EnemyManager : MonoBehaviour
{
    public GameObject[] waypoints;
    public List<GameObject> enemies;
    public static EnemyManager instance;

    private void Awake()
    {
        instance = this;
        waypoints = GameObject.FindGameObjectsWithTag("Waypoint");
    }

    private void Start()
    {
        GameObject[] startEnemies = GameObject.FindGameObjectsWithTag("Enemy");

        HUDManager.instance.UpdateProgress(startEnemies.Length);
    }
}
