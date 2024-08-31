using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.AI;
using UnityEngine.Events;



public class BaseAI : MonoBehaviour, IDamageable
{
    // Behaviors and stats
    [Header("------ Enemy Stats ------")]
    [Range(1, 100)][SerializeField] float HP;
    [Range(1, 180)][SerializeField] int viewCone;
    [Range(1, 100)][SerializeField] int targetFaceSpeed;
    [SerializeField] float walkSpeed;
    [SerializeField] float sprintMultipler;

    [Header("------ Combat ------")]
    [Range(0, 100)][SerializeField] float shootRate;
    [SerializeField] EnemyCombat enemyCombat;
    [Range(0.1f, 5f)][SerializeField] float detectionDelay;
    public bool playerdetected;

    [Header("------ NavMesh Components ------")]
    public NavMeshAgent agent; // READ --> Keep it public so the roam scripts can access this variable
    [SerializeField] float stoppingDist;
    [SerializeField] bool isASniper;
    [SerializeField] int findNearestEnemies;

    [Header("------ Colliders ------")]
    [SerializeField] CapsuleCollider hitCol;
    [SerializeField] SphereCollider detectCol;

    [Header("------ Model ------")]
    [SerializeField] Renderer model;
    [SerializeField] Transform headPosition;


    [Header("------ Animation ------")]
    [SerializeField] Animator enemyAnim;
    [SerializeField] float animSpeedTransition;




    // private Combat variables
    bool isSprinting;
    bool isShooting;
    bool isDead;
    float angleToPlayer;
    Vector3 playerDir;
    Vector3 oppositeDir;

    //private Patrol variables
    bool playerInRange;
    bool isMoving;
    Vector3 enemyPos;
    Vector3 destinationPoint;
    Coroutine returnOriginalPositionCot;

    void Start()
    {
        EnemyManager.instance.enemies.Add(gameObject);
        enemyPos = transform.position;
        agent.stoppingDistance = stoppingDist;
        agent.speed = walkSpeed;
    }

    void Update()
    {
        if (!isDead)
        {
            float animVelocity = agent.velocity.normalized.magnitude;
            enemyAnim.SetFloat("Speed", Mathf.Lerp(enemyAnim.GetFloat("Speed"), animVelocity, Time.deltaTime * animSpeedTransition));


            if (playerInRange && !canSeePlayer())
            {
                patrol();
            }
            else if (!playerInRange)
            {
                patrol();
            }
        }
    }

    public virtual void patrol()
    {
        if (isMoving)
        {
            agent.stoppingDistance = 0;
            returnOriginalPositionCot = StartCoroutine(returntoOriginPos(1));
        }
    }

    public virtual bool canSeePlayer()
    {
        playerDir = GameManager.instance.playerBodyPositions.GetPlayerCenter() - headPosition.position;
        angleToPlayer = Vector3.Angle(playerDir, transform.forward);

        Debug.DrawRay(headPosition.position, playerDir);

        RaycastHit hit;

        if (Physics.Raycast(headPosition.position, playerDir, out hit))
        {
            if (hit.transform.CompareTag("Player") && angleToPlayer <= viewCone)
            {
                if (playerdetected)
                {
                    if (returnOriginalPositionCot != null)
                        StopCoroutine(returnOriginalPositionCot);
                    if (!isASniper)
                    {
                        if (agent.isOnNavMesh)
                            agent.SetDestination(GameManager.instance.player.transform.position);
                    }
                    enemyCombat.AimAt(GameManager.instance.playerBodyPositions.GetPlayerCenter());

                    if (!isShooting)
                    {
                        StartCoroutine(shoot());
                    }

                    if (agent.isOnNavMesh)
                    {
                        isMoving = true;
                        if (!isSprinting)
                        {
                            agent.speed *= sprintMultipler;
                            isSprinting = true;
                        }
                        agent.stoppingDistance = stoppingDist;
                    }

                    if (isASniper)
                    {
                        if (agent.isOnNavMesh)
                        {

                            if (Vector3.Distance(GameManager.instance.player.transform.position, transform.position) < agent.stoppingDistance - 2)
                            {
                                agent.updateRotation = false;
                                agent.SetDestination((-transform.forward * (agent.stoppingDistance + 2)) + transform.position);;
                            }
                            if (Vector3.Distance(transform.position, GameManager.instance.player.transform.position) <= detectCol.radius * 4)
                            {
                                faceTarget();
                            }
                        }
                        if (!agent.isOnNavMesh)
                        {
                            if (Vector3.Distance(transform.position, GameManager.instance.player.transform.position) <= detectCol.radius * 4)
                            {
                                faceTarget();
                            }
                        }
                    }
                    else
                    {
                        if (agent.isOnNavMesh)
                        {
                            if (agent.remainingDistance < agent.stoppingDistance)
                            {
                                faceTarget();
                            }
                        }
                        else if (!agent.isOnNavMesh)
                        {
                            if (Vector3.Distance(transform.position, GameManager.instance.player.transform.position) <= detectCol.radius * 4)
                            {
                                faceTarget();
                            }
                        }
                    }
                    return true;
                }
                else if (!playerdetected)
                {
                    faceTarget();
                    StartCoroutine(DetectionDelay());
                }
            }
        }
        playerdetected = false;
        isSprinting = false;
        if (agent.isOnNavMesh)
        {
            agent.speed = walkSpeed;
            agent.stoppingDistance = 0;
        }
        return false;
    }

    IEnumerator DetectionDelay()
    {
        if (agent.isOnNavMesh)
        {
            agent.velocity = Vector3.zero;
        }

        float currentDelayTime = 0;

        while (currentDelayTime < detectionDelay)
        {
            currentDelayTime += Time.deltaTime;
            faceTarget();
            enemyCombat.AimAt(GameManager.instance.playerBodyPositions.GetPlayerCenter());

            yield return null;
        }

        playerdetected = true;
    }

    IEnumerator returntoOriginPos(int delay)
    {
        if (agent.isOnNavMesh)
        {
            if (agent.remainingDistance <= 0.01f)
            {
                destinationPoint = enemyPos;
                yield return new WaitForSeconds(delay);
                agent.SetDestination(destinationPoint);
                isMoving = false;
            }
        }
    }


    void faceTarget()
    {
        Quaternion rot = Quaternion.LookRotation(new Vector3(playerDir.x, 0, playerDir.z));
        transform.rotation = Quaternion.Lerp(transform.rotation, rot, Time.deltaTime * targetFaceSpeed);
    }

    IEnumerator shoot()
    {
        if (!isDead)
        {
            isShooting = true;
            enemyCombat.AttackStarted();
            enemyAnim.SetTrigger("Shooting");
            yield return new WaitForSeconds(shootRate);
            enemyAnim.SetTrigger("Shooting");

            enemyCombat.AttackCanceled();

            isShooting = false;
        }
    }

    public virtual void TakeDamage(float damage)
    {
        HP -= damage;

        if (HP <= 0)
        {
            isDead = true;
            hitCol.enabled = false;
            detectCol.enabled = false;
            if (agent.isOnNavMesh)
                agent.enabled = false;
            StopAllCoroutines();
            model.material.color = Color.red;
            enemyAnim.SetBool("isDead", true);
            enemyAnim.SetLayerWeight(1, 0);
            StartCoroutine(WaitBeforeDestroying());
            HUDManager.instance.UpdateProgress(-1);
            OnDie.Invoke();
            EnemyManager.instance.enemies.Remove(gameObject);
        }
        else
        {
            enemyAnim.SetLayerWeight(1, Mathf.Lerp(0, 1, 10 * Time.deltaTime));
            enemyAnim.SetTrigger("Damage");
            playerdetected = true;
            isShooting = false;
            if (returnOriginalPositionCot != null)
                StopCoroutine(returnOriginalPositionCot);
            StartCoroutine(flashRed());
            if (!isASniper)
            {
                if (agent.isOnNavMesh)
                {
                    agent.SetDestination(GameManager.instance.player.transform.position);
                    isMoving = true;
                }
            }
            else if (isASniper)
            {
                CallForBackup();
            }
            enemyAnim.SetLayerWeight(1, Mathf.Lerp(1, 0, 10 * Time.deltaTime));
        }
    }

    IEnumerator WaitBeforeDestroying()
    {
        yield return new WaitForSeconds(8);
        Destroy(gameObject);
    }

    void CallForBackup()
    {
        for (int i = 0; i < EnemyManager.instance.enemies.Count; i++)
        {
            if (EnemyManager.instance.enemies[i] != null)
            {
                if (Vector3.Distance(transform.position, EnemyManager.instance.enemies[i].transform.position) < findNearestEnemies)
                {
                    BaseAI tempEnemy = EnemyManager.instance.enemies[i].GetComponent<BaseAI>();

                    if (tempEnemy != null)
                    {
                        if (!tempEnemy.isASniper && tempEnemy != this)
                        {
                            if (tempEnemy.agent.isOnNavMesh)
                            {
                                tempEnemy.agent.SetDestination(GameManager.instance.player.transform.position);
                                tempEnemy.isMoving = true;
                            }
                        }
                    }
                }
            }

        }
    }


    IEnumerator flashRed()
    {
        model.material.color = Color.red;
        yield return new WaitForSeconds(0.1f);
        model.material.color = Color.white;
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.isTrigger)
            return;

        if (other.CompareTag("Player"))
        {
            playerInRange = true;
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.isTrigger)
            return;

        if (other.CompareTag("Player"))
        {
            playerInRange = false;
        }
    }

    private void OnDrawGizmosSelected()
    {
        Gizmos.color = Color.cyan;
        Gizmos.DrawWireSphere(transform.position, findNearestEnemies);
    }

    private UnityEvent OnDie = new UnityEvent();

    public void SubscribeOnDie(UnityAction action) { OnDie.AddListener(action); }
    public void UnsubscribeOnDie(UnityAction action) { OnDie.RemoveListener(action); }
}
