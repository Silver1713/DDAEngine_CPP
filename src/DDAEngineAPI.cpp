#include "DDAEngineAPI.h"
#include "DDAEngine.hpp"
#include <cstring>
#include <string>
#include <memory>
#include <Windows.h>
static std::unique_ptr<std::string> lastAllocatedString;
static std::unique_ptr<std::string> lastHintsString;


#define NATIVE_DEBUG(content) OutputDebugString(content)

extern "C" {

	DDAEngine* instance;

	DDA_API void AllocateConsole()
	{

		if (!::AttachConsole(ATTACH_PARENT_PROCESS))
			::AllocConsole();


		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
		freopen_s(&fp, "CONIN$", "r", stdin);


		// (optional) give the console window a title
		SetConsoleTitleW(L"Unity Native Plug-in Output");

		// 3. Set the console mode to allow VT sequences (for color output)
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode;
		if (GetConsoleMode(hConsole, &mode)) {
			SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
		else {
			printf("Failed to get console mode\n");
		}







	}

	DDA_API int DDA_Load()
	{
		AllocateConsole();
		instance = &DDAEngine::getInstance();
		if (instance) return 0;
		return 1;
	}


	DDA_API int DDA_LoadConfig(char* json)
	{
		if (!instance)
		{
			DDA_Load();
		}
		NATIVE_DEBUG("Load Param config\n");
		ParameterConfig paramConfig;
		if (paramConfig.loadFromFile("../config/shooter_game_config.json")) {
			NATIVE_DEBUG("Parameter configuration loaded successfully!\n");
			char message[2000];
			snprintf(message, 2000, "Found %d parameter groups\n", paramConfig.getParameterGroups().size());
			NATIVE_DEBUG(message);
		}
		else {
			NATIVE_DEBUG("Loading params failed\n");
			paramConfig = ParameterConfig::createDefaultConfig();
		}
		return 0;
	}

	DDA_API int DDA_INIT()
	{
		return 0;
	}
	
	DDA_API void DDA_Initialize() {
		DDAEngine::getInstance().initialize();
	}

	DDA_API void DDA_Shutdown() {
		FreeConsole();
	}

	DDA_API void DDA_CollectMetric(const char* metricName, float value) {
		try {
			MetricManager::getInstance()->pushMetric(metricName, value);
		}
		catch (...) {

		}
	}

	DDA_API void DDA_CollectMetricInt(const char* metricName, int value) {
		try {
			MetricManager::getInstance()->pushMetric(metricName, value);
		}
		catch (...) {

		}
	}

	DDA_API void DDA_SubmitLevelMetrics(const char* jsonMetricMatrix) {
		if (jsonMetricMatrix) {
			DDAEngine::getInstance().collectLevelMetrics(jsonMetricMatrix);
		}
	}

	DDA_API void DDA_EvolveParameters() {
		DDAEngine::getInstance().evolveParameters();
	}

	DDA_API void DDA_GetAIParameters(AIParametersC* outParams) {
		if (outParams) {
			auto params = DDAEngine::getInstance().getCurrentParameters();
			auto aiParams = params.getAIParameters();
			outParams->aggressiveness = aiParams.aggressiveness;
			outParams->reactionTime = aiParams.reactionTime;
			outParams->accuracy = aiParams.accuracy;
			outParams->movementSpeed = aiParams.movementSpeed;
			outParams->detectionRange = aiParams.detectionRange;
			outParams->attackFrequency = aiParams.attackFrequency;
		}
	}

	DDA_API void DDA_GetPCGParameters(PCGParametersC* outParams) {
		if (outParams) {
			auto params = DDAEngine::getInstance().getCurrentParameters();
			auto pcgParams = params.getPCGParameters();
			outParams->enemyDensity = pcgParams.enemyDensity;
			outParams->powerUpFrequency = pcgParams.powerUpFrequency;
			outParams->obstacleComplexity = pcgParams.obstacleComplexity;
			outParams->pathBranching = pcgParams.pathBranching;
			outParams->hazardIntensity = pcgParams.hazardIntensity;
			outParams->minEnemiesPerRoom = pcgParams.minEnemiesPerRoom;
			outParams->maxEnemiesPerRoom = pcgParams.maxEnemiesPerRoom;
		}
	}

	DDA_API float DDA_GetDifficultyMultiplier() {
		auto params = DDAEngine::getInstance().getCurrentParameters();
		return params.getValue<float>("Global.difficultyMultiplier", 1.0f);
	}

	DDA_API void DDA_SetMode(DDAMode mode) {
		DDAEngine::getInstance().setMode(static_cast<::DDAMode>(mode));
	}

	DDA_API DDAMode DDA_GetMode() {
		return static_cast<DDAMode>(DDAEngine::getInstance().getMode());
	}

	DDA_API void DDA_SetEvolutionEnabled(int enabled) {
		DDAEngine::getInstance().setEvolutionEnabled(enabled != 0);
	}

	DDA_API int DDA_IsEvolutionEnabled() {
		return DDAEngine::getInstance().isEvolutionEnabledStatus() ? 1 : 0;
	}

	DDA_API void DDA_UpdateAdaptive(float deltaTime) {
		DDAEngine::getInstance().applyAdaptiveAdjustment(deltaTime);
	}

	DDA_API const char* DDA_GetLevelGenerationHints() {
		try {
			auto hints = DDAEngine::getInstance().getLevelGenerationHints();
			lastHintsString = std::make_unique<std::string>(hints.dump());
			return lastHintsString->c_str();
		}
		catch (...) {
			return "{}";
		}
	}

	DDA_API const char* DDA_ExportParametersJson() {
		try {
			lastAllocatedString = std::make_unique<std::string>(
				DDAEngine::getInstance().exportParametersAsJson()
			);
			return lastAllocatedString->c_str();
		}
		catch (...) {
			return "{}";
		}
	}

	DDA_API void DDA_ImportParametersJson(const char* json) {
		if (json) {

			DDAEngine::getInstance().importParametersFromJson(json);
		}
	}

	DDA_API float DDA_GetPlayerSkillLevel() {
		return DDAEngine::getInstance().calculatePlayerSkillLevel();
	}

	DDA_API void DDA_SetIdealMetrics(float completionTime, float deathRate, float accuracy) {
		DDAEngine::getInstance().setIdealMetrics(completionTime, deathRate, accuracy);
	}

	DDA_API void DDA_Reset() {
		DDAEngine::getInstance().resetToDefaults();
	}

	DDA_API void DDA_FreeString(const char* str) {

	}

}