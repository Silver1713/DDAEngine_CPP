using System;
using System.Runtime.InteropServices;
using UnityEngine;

using static DDAEngineWrapper;
using System.Collections.Generic;
// Unity C# wrapper for DDAEngine
public class DDAEngineWrapper : MonoBehaviour
{
    #region DLL Imports
    [DllImport("DDAEngine")]
    private static extern void DDA_Initialize();

    [DllImport("DDAEngine")]
    private static extern void DDA_Shutdown();

    [DllImport("DDAEngine")]
    private static extern void DDA_CollectMetric(string metricName, float value);

    [DllImport("DDAEngine")]
    private static extern void DDA_CollectMetricInt(string metricName, int value);

    [DllImport("DDAEngine")]
    private static extern void DDA_SubmitLevelMetrics(string jsonMetricMatrix);

    [DllImport("DDAEngine")]
    private static extern void DDA_EvolveParameters();

    [DllImport("DDAEngine")]
    private static extern void DDA_GetAIParameters(out AIParameters outParams);

    [DllImport("DDAEngine")]
    private static extern void DDA_GetPCGParameters(out PCGParameters outParams);

    [DllImport("DDAEngine")]
    private static extern float DDA_GetDifficultyMultiplier();

    [DllImport("DDAEngine")]
    private static extern void DDA_SetMode(int mode);

    [DllImport("DDAEngine")]
    private static extern int DDA_GetMode();

    [DllImport("DDAEngine")]
    private static extern void DDA_SetEvolutionEnabled(int enabled);

    [DllImport("DDAEngine")]
    private static extern int DDA_IsEvolutionEnabled();

    [DllImport("DDAEngine")]
    private static extern void DDA_UpdateAdaptive(float deltaTime);

    [DllImport("DDAEngine")]
    private static extern IntPtr DDA_GetLevelGenerationHints();

    [DllImport("DDAEngine")]
    private static extern IntPtr DDA_ExportParametersJson();

    [DllImport("DDAEngine")]
    private static extern void DDA_ImportParametersJson(string json);

    [DllImport("DDAEngine")]
    private static extern float DDA_GetPlayerSkillLevel();

    [DllImport("DDAEngine")]
    private static extern void DDA_SetIdealMetrics(float completionTime, float deathRate, float accuracy);

    [DllImport("DDAEngine")]
    private static extern void DDA_Reset();

    [DllImport("DDAEngine")]
    private static extern void DDA_FreeString(IntPtr str);
    #endregion

    #region Structures
    [StructLayout(LayoutKind.Sequential)]
    public struct AIParameters
    {
        public float aggressiveness;
        public float reactionTime;
        public float accuracy;
        public float movementSpeed;
        public float detectionRange;
        public float attackFrequency;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct PCGParameters
    {
        public float enemyDensity;
        public float powerUpFrequency;
        public float obstacleComplexity;
        public float pathBranching;
        public float hazardIntensity;
        public int minEnemiesPerRoom;
        public int maxEnemiesPerRoom;
    }

    public enum DDAMode
    {
        Adaptive = 0,
        Fixed = 1,
        Learning = 2
    }
    #endregion

    #region Singleton
    private static DDAEngineWrapper instance;
    public static DDAEngineWrapper Instance
    {
        get
        {
            if (instance == null)
            {
                instance = FindObjectOfType<DDAEngineWrapper>();
                if (instance == null)
                {
                    GameObject go = new GameObject("DDAEngine");
                    instance = go.AddComponent<DDAEngineWrapper>();
                    DontDestroyOnLoad(go);
                }
            }
            return instance;
        }
    }
    #endregion

    #region Level Metrics Class
    [Serializable]
    public class LevelMetrics
    {
        public string level_id;
        public float completion_time;
        public float accuracy;
        public int player_deaths;
        public int enemies_killed;
        public float damage_taken;
        public int powerups_collected;
        public float distance_traveled;

        public LevelMetrics(string levelId)
        {
            level_id = levelId;
            completion_time = 0f;
            accuracy = 0f;
            player_deaths = 0;
            enemies_killed = 0;
            damage_taken = 0f;
            powerups_collected = 0;
            distance_traveled = 0f;
        }
    }
    #endregion

    private LevelMetrics currentLevelMetrics;
    private float levelStartTime;

    void Awake()
    {
        if (instance != null && instance != this)
        {
            Destroy(gameObject);
            return;
        }

        instance = this;
        DontDestroyOnLoad(gameObject);

        DDA_Initialize();
        DDA_SetIdealMetrics(300f, 0.2f, 0.7f);
        DDA_SetMode((int)DDAMode.Adaptive);
        DDA_SetEvolutionEnabled(1);
    }

    void Update()
    {
        // Update adaptive parameters smoothly
        DDA_UpdateAdaptive(Time.deltaTime);
    }

    void OnDestroy()
    {
        if (instance == this)
        {
            DDA_Shutdown();
        }
    }

    #region Public API
    public void StartLevel(string levelId)
    {
        currentLevelMetrics = new LevelMetrics(levelId);
        levelStartTime = Time.time;
    }

    public void EndLevel()
    {
        if (currentLevelMetrics != null)
        {
            currentLevelMetrics.completion_time = Time.time - levelStartTime;

            string json = JsonUtility.ToJson(currentLevelMetrics);
            DDA_SubmitLevelMetrics(json);

            // Trigger evolution after every 5 levels
            if (UnityEngine.Random.Range(0, 5) == 0)
            {
                DDA_EvolveParameters();
            }
        }
    }

    public void RecordPlayerDeath()
    {
        if (currentLevelMetrics != null)
        {
            currentLevelMetrics.player_deaths++;
        }
        DDA_CollectMetricInt("player_deaths", 1);
    }

    public void RecordEnemyKill()
    {
        if (currentLevelMetrics != null)
        {
            currentLevelMetrics.enemies_killed++;
        }
        DDA_CollectMetricInt("enemies_killed", 1);
    }

    public void RecordDamage(float damage)
    {
        if (currentLevelMetrics != null)
        {
            currentLevelMetrics.damage_taken += damage;
        }
        DDA_CollectMetric("damage_taken", damage);
    }

    public void RecordShot(bool hit)
    {
        if (currentLevelMetrics != null && hit)
        {
            float currentAccuracy = currentLevelMetrics.accuracy;
            int totalShots = currentLevelMetrics.enemies_killed + 1;
            currentLevelMetrics.accuracy = (currentAccuracy * (totalShots - 1) + (hit ? 1f : 0f)) / totalShots;
        }
        DDA_CollectMetric("accuracy", hit ? 1f : 0f);
    }

    public AIParameters GetAIParameters()
    {
        AIParameters aiParams;
        DDA_GetAIParameters(out aiParams);
        return aiParams;
    }

    public PCGParameters GetPCGParameters()
    {
        PCGParameters pcgParams;
        DDA_GetPCGParameters(out pcgParams);
        return pcgParams;
    }

    public float GetDifficultyMultiplier()
    {
        return DDA_GetDifficultyMultiplier();
    }

    public float GetPlayerSkillLevel()
    {
        return DDA_GetPlayerSkillLevel();
    }

    public string GetLevelGenerationHints()
    {
        IntPtr ptr = DDA_GetLevelGenerationHints();
        string result = Marshal.PtrToStringAnsi(ptr);
        return result;
    }

    public void SetMode(DDAMode mode)
    {
        DDA_SetMode((int)mode);
    }

    public void SetEvolutionEnabled(bool enabled)
    {
        DDA_SetEvolutionEnabled(enabled ? 1 : 0);
    }
    #endregion
}

// Example Enemy AI Controller using DDA parameters
public class EnemyAI : MonoBehaviour
{
    private AIParameters aiParams;
    private float nextUpdateTime;

    void Start()
    {
        UpdateAIParameters();
    }

    void Update()
    {
        if (Time.time >= nextUpdateTime)
        {
            UpdateAIParameters();
            nextUpdateTime = Time.time + 5f; // Update every 5 seconds
        }

        // Use AI parameters to control behavior
        float moveSpeed = aiParams.movementSpeed * 5f; // Base speed 5 units/sec
        float detectionRange = aiParams.detectionRange;

        // Example: Move towards player if within detection range
        GameObject player = GameObject.FindWithTag("Player");
        if (player != null)
        {
            float distance = Vector3.Distance(transform.position, player.transform.position);
            if (distance <= detectionRange)
            {
                Vector3 direction = (player.transform.position - transform.position).normalized;
                transform.position += direction * moveSpeed * Time.deltaTime;

                // Attack based on frequency
                if (UnityEngine.Random.value < aiParams.attackFrequency * Time.deltaTime)
                {
                    Attack(player);
                }
            }
        }
    }

    void UpdateAIParameters()
    {
        aiParams = DDAEngineWrapper.Instance.GetAIParameters();
    }

    void Attack(GameObject target)
    {
        // Attack logic with accuracy modifier
        if (UnityEngine.Random.value < aiParams.accuracy)
        {
            // Hit the player
            Debug.Log("Enemy hit the player!");
        }
        else
        {
            // Miss
            Debug.Log("Enemy missed!");
        }
    }
}

// Example Level Generator using DDA parameters
public class LevelGenerator : MonoBehaviour
{
    public GameObject enemyPrefab;
    public GameObject powerupPrefab;
    public GameObject obstaclePrefab;

    public void GenerateLevel()
    {
        PCGParameters pcgParams = DDAEngineWrapper.Instance.GetPCGParameters();
        string hintsJson = DDAEngineWrapper.Instance.GetLevelGenerationHints();
        var hints = JsonUtility.FromJson<Dictionary<string, object>>(hintsJson);

        // Generate enemies based on density
        int numRooms = 10;
        for (int i = 0; i < numRooms; i++)
        {
            int enemiesInRoom = UnityEngine.Random.Range(
                pcgParams.minEnemiesPerRoom,
                pcgParams.maxEnemiesPerRoom + 1
            );

            for (int j = 0; j < enemiesInRoom; j++)
            {
                Vector3 spawnPos = GetRandomRoomPosition(i);
                Instantiate(enemyPrefab, spawnPos, Quaternion.identity);
            }
        }

        // Generate powerups
        int powerupCount = Mathf.RoundToInt(numRooms * pcgParams.powerUpFrequency * 3);
        for (int i = 0; i < powerupCount; i++)
        {
            Vector3 spawnPos = GetRandomPosition();
            Instantiate(powerupPrefab, spawnPos, Quaternion.identity);
        }

        // Generate obstacles based on complexity
        int obstacleCount = Mathf.RoundToInt(50 * pcgParams.obstacleComplexity);
        for (int i = 0; i < obstacleCount; i++)
        {
            Vector3 spawnPos = GetRandomPosition();
            Instantiate(obstaclePrefab, spawnPos, Quaternion.identity);
        }

        Debug.Log($"Generated level for {hints["level_type"]} player (skill: {hints["recommended_difficulty"]})");
    }

    Vector3 GetRandomRoomPosition(int roomIndex)
    {
        // Implement your room-based position logic
        return new Vector3(
            roomIndex * 20f + UnityEngine.Random.Range(-8f, 8f),
            0f,
            UnityEngine.Random.Range(-8f, 8f)
        );
    }

    Vector3 GetRandomPosition()
    {
        return new Vector3(
            UnityEngine.Random.Range(-100f, 100f),
            0f,
            UnityEngine.Random.Range(-100f, 100f)
        );
    }
}