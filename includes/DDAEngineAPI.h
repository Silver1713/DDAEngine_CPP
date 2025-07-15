#ifndef DDA_ENGINE_API_H
#define DDA_ENGINE_API_H
#include <cstdio>
#include <iosfwd>
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef DDA_ENGINE_EXPORTS
#define DDA_API __declspec(dllexport)
#else
#define DDA_API __declspec(dllimport)
#endif
#else
#define DDA_API
#endif

	typedef enum {
		DDA_MODE_ADAPTIVE = 0,
		DDA_MODE_FIXED = 1,
		DDA_MODE_LEARNING = 2
	} DDAMode;

	typedef struct {
		float aggressiveness;
		float reactionTime;
		float accuracy;
		float movementSpeed;
		float detectionRange;
		float attackFrequency;
	} AIParametersC;

	typedef struct {
		float enemyDensity;
		float powerUpFrequency;
		float obstacleComplexity;
		float pathBranching;
		float hazardIntensity;
		int minEnemiesPerRoom;
		int maxEnemiesPerRoom;
	} PCGParametersC;

	
	DDA_API int DDA_Load();

	DDA_API int DDA_LoadConfig(char* json);

	DDA_API int DDA_INIT();

	DDA_API void DDA_Initialize();

	DDA_API void DDA_Shutdown();

	DDA_API void DDA_CollectMetric(const char* metricName, float value);

	DDA_API void DDA_CollectMetricInt(const char* metricName, int value);

	DDA_API void DDA_SubmitLevelMetrics(const char* jsonMetricMatrix);

	DDA_API void DDA_EvolveParameters();

	DDA_API void DDA_GetAIParameters(AIParametersC* outParams);

	DDA_API void DDA_GetPCGParameters(PCGParametersC* outParams);

	DDA_API float DDA_GetDifficultyMultiplier();

	DDA_API void DDA_SetMode(DDAMode mode);

	DDA_API void DDA_AdvanceEngine();

	DDA_API DDAMode DDA_GetMode();

	DDA_API void DDA_SetEvolutionEnabled(int enabled);

	DDA_API int DDA_IsEvolutionEnabled();

	DDA_API void DDA_UpdateAdaptive(float deltaTime);

	DDA_API const char* DDA_GetLevelGenerationHints();

	DDA_API const char* DDA_ExportParametersJson();

	DDA_API void DDA_ImportParametersJson(const char* json);

	DDA_API float DDA_GetPlayerSkillLevel();


	DDA_API void DDA_SetAnyIdealMetric(const char* metricName, float idealValue);

	DDA_API void DDA_SetIdealMetrics(float completionTime, float deathRate, float accuracy);

	DDA_API void DDA_Reset();

	DDA_API void DDA_FreeString(const char* str);

#ifdef __cplusplus
}
#endif

#endif