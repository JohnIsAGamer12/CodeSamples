using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AmbushTriggers : MonoBehaviour
{
    [Header("----- Particle System -----")]
    public GameObject particlePoofPrefab;

    [Header("----- Spawners Components -----")]
    public GameObject[] _enemies;

    public bool isAPickUp;
    
    GameObject[] poofParticle;

    bool playerHasTriggeredTrap;
    int maxEnemiesInScene;
    int currentEnemiesInScene;


    private void Start()
    {
        maxEnemiesInScene = _enemies.Length;
    }

    // Update is called once per frame
    void Update()
    {
        if (!isAPickUp)
        {
            if (playerHasTriggeredTrap && maxEnemiesInScene > 0)
            {
                SpawnEnemies();
            }
        }
    }

    public void SpawnEnemies()
    {
        InstantiateParticles();
        StartCoroutine(PoofBeforeAppearing(1));
    }

    public void InstantiateParticles()
    {
        poofParticle = new GameObject[maxEnemiesInScene];

        for (int i = 0; i < _enemies.Length; ++i)
        {
            if (_enemies[i] != null)
            {
                GameObject tempParticlePrefab = Instantiate(particlePoofPrefab, _enemies[i].transform.position,
                                                                                _enemies[i].transform.rotation);
                poofParticle[i] = tempParticlePrefab;
            }
        }
    }

    public void DestroyParticlesGameObjects()
    {
        for (int i = 0; i < poofParticle.Length; ++i)
        {
            if (poofParticle[i] != null)
            {
                Destroy(poofParticle[i]);
            }
        }
        poofParticle = null;
    }

    IEnumerator PoofBeforeAppearing(float delay)
    {
        for (int i = 0; i < poofParticle.Length; ++i)
        {
            ParticleSystem poofFX = poofParticle[i].GetComponent<ParticleSystem>();

            poofFX.Play();
        }
        ActivateEnemies();
        yield return new WaitForSeconds(delay);
        DestroyParticlesGameObjects();
    }

    public void ActivateEnemies()
    {
        if (currentEnemiesInScene < maxEnemiesInScene)
        {
            for (int i = 0; i < _enemies.Length; ++i)
            {
                if (_enemies[i] == null)
                    continue;

                _enemies[i].SetActive(true);
                currentEnemiesInScene++;
            }
            playerHasTriggeredTrap = false;
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            playerHasTriggeredTrap = true;
        }
    }
}
