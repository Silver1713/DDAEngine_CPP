# Adaptive Game AI Framework - Technical Report

## Overview

Our Adaptive Game AI Framework is a comprehensive system that combines Dynamic Difficulty Adjustment (DDA) with behavioral AI systems to create responsive, intelligent gameplay experiences. The framework integrates a C++ DDA engine with Unity-based AI systems, providing:

- **Hybrid AI Architecture**: Combines Finite State Machines (FSM) for behavioral AI with genetic algorithm-based difficulty adaptation
- **Real-time Parameter Evolution**: Uses genetic algorithms to evolve AI and level generation parameters based on player performance
- **Procedural Content Generation**: Dynamic room and enemy generation that adapts to player skill
- **Comprehensive Metrics System**: Tracks player performance across multiple dimensions for intelligent adaptation
- **Modular Integration**: Clean separation between C++ optimization engine and C# gameplay systems
- **Multi-mode Operation**: Supports adaptive, fixed, and learning modes for different gameplay scenarios

## Design Philosophy

### Core Principles

The DDAEngine is built on several fundamental design principles that guide its architecture and implementation:

#### 1. **Player-Centric Adaptation**
The system prioritizes player experience by continuously monitoring performance metrics and adjusting game difficulty to maintain an optimal challenge level. Rather than forcing players into predefined difficulty categories, the engine creates a personalized experience that evolves with the player's skill progression.

#### 2. **Non-Intrusive Integration**
The engine is designed as a separate module that can be integrated into existing game architectures without requiring significant refactoring. Through its C API and plugin architecture, games can adopt DDA capabilities with minimal code changes.

#### 3. **Data-Driven Evolution**
All adaptation decisions are based on quantifiable metrics rather than heuristics. The system collects, analyzes, and responds to actual player behavior data, ensuring that difficulty adjustments are grounded in measurable performance indicators.

#### 4. **Smooth Transitions**
Recognizing that abrupt difficulty changes can break immersion, the engine implements sophisticated interpolation algorithms to ensure all parameter adjustments happen gradually and imperceptibly during gameplay.

#### 5. **Modularity and Extensibility**
The architecture follows SOLID principles, particularly the Open/Closed Principle, allowing developers to extend the system with new metrics, parameters, or adaptation strategies without modifying core functionality.

## Core Techniques

### 1. Genetic Algorithm for Parameter Evolution

The engine employs a genetic algorithm (GA) to evolve optimal game parameters based on player performance. This evolutionary approach offers several advantages:

```cpp
// Population-based evolution with configurable parameters
GeneticAlgorithm(size_t popSize = 50, size_t elite = 5, 
                 float mutRate = 0.1f, float crossRate = 0.7f)
```

**Key GA Components:**
- **Population Management**: Maintains 50 individuals (parameter sets) that compete based on fitness
- **Tournament Selection**: Uses 3-way tournament selection to choose parents, balancing exploration and exploitation
- **Uniform Crossover**: Each gene has 50% probability of coming from either parent
- **Adaptive Mutation**: 10% mutation rate with parameter-specific constraints
- **Elitism**: Preserves top 5 performers to ensure solution quality never degrades

### 2. Multi-Metric Fitness Evaluation

The fitness function combines multiple performance indicators with weighted contributions:

```cpp
float calculateFitness(const DDAParameters& params, const MetricManager& metrics) {
    float fitness = 100.0f;
    
    // Death rate component (30% weight)
    float deathPenalty = std::abs(deathRate - idealDeathRate) * 30.0f;
    
    // Completion time component (25% weight)
    float timePenalty = normalizeMetric(avgTime, 60.0f, 600.0f, idealCompletionTime) * 25.0f;
    
    // Accuracy component (20% weight)
    float accuracyPenalty = std::abs(avgAccuracy - idealAccuracy) * 20.0f;
    
    // Balance and parameter penalties
    fitness -= (deathPenalty + timePenalty + accuracyPenalty + balancePenalty);
    
    return std::max(0.0f, fitness);
}
```

### 3. Real-Time Parameter Interpolation

To ensure smooth gameplay experience, the engine uses frame-based linear interpolation:

```cpp
void smoothParameterTransition(float deltaTime) {
    float lerpSpeed = 0.5f;
    float t = std::min(1.0f, lerpSpeed * deltaTime);
    
    // Smooth interpolation for each parameter
    currentParams = lerp(currentParams, targetParams, t);
}
```

### 4. Template-Based Type-Safe Metrics System

The metrics system leverages C++20 concepts for compile-time type safety:

```cpp
template <typename T>
concept IsAveragable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { a / b } -> std::convertible_to<T>;
};

template <typename T>
struct DataStore {
    std::deque<T> data;
    T average() const requires (IsAveragable<T>);
};
```

### 5. Sliding Window Data Analysis

The system implements configurable sliding windows for recent performance focus:
- Maintains recent performance history (default: last 30 data points)
- Automatically removes outdated metrics
- Supports both windowed and cumulative analysis modes

### 6. Player Skill Level Calculation

Multi-factor skill assessment algorithm:

```cpp
float calculatePlayerSkillLevel() const {
    float skillLevel = 0.5f;  // Base skill level
    
    // Time-based performance (30% weight)
    skillLevel = skillLevel * 0.7f + timeScore * 0.3f;
    
    // Accuracy metrics (30% weight) 
    skillLevel = skillLevel * 0.7f + accuracy * 0.3f;
    
    // Death rate analysis (20% weight)
    float deathScore = 1.0f - std::min(1.0f, deathRate);
    skillLevel = skillLevel * 0.8f + deathScore * 0.2f;
    
    return std::max(0.0f, std::min(1.0f, skillLevel));
}
```

### 7. Level Generation Hints System

The engine provides contextual hints for procedural generation:

```cpp
nlohmann::json getLevelGenerationHints() const {
    hints["recommended_difficulty"] = calculatePlayerSkillLevel();
    
    if (skillLevel < 0.3f) hints["level_type"] = "tutorial";
    else if (skillLevel < 0.6f) hints["level_type"] = "standard";
    else if (skillLevel < 0.85f) hints["level_type"] = "challenging";
    else hints["level_type"] = "expert";
}
```

## System Architecture

### Complete Framework Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Unity/C# Game Layer                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────┐      ┌────────────────────┐            │
│  │ DungeonManager  │      │  DDAEngineWrapper  │            │
│  │   (Singleton)   │◄─────┤    (Singleton)     │            │
│  │ • Grid System   │      │ • Metric Collection│            │
│  │ • Room Loading  │      │ • Parameter Fetch  │            │
│  └────────┬────────┘      └──────────┬─────────┘            │
│           │                           │                     │
│           ▼                           ▼                     │
│  ┌─────────────────┐            P/Invoke API                │
│  │  RoomManager    │                 │                      │
│  │ • PCG Generation│                 ▼                      │
│  │ • Enemy Spawning│      ┌──────────────────────┐          │
│  │ • Tile Placement│      │  DDAEngineAPI (C)    │          │
│  └────────┬────────┘      │ • Type Marshalling   │          │
│           │               │ • Memory Management  │          │
│           ▼               └──────────┬───────────┘          │
│  ┌─────────────────────────┐         │                      │
│  │    AI State Machines    │         ▼                      │
│  ├─────────────────────────┤  ┌───────────────────┐         │
│  │ PlayerStateManager      │  │ DDAEngine Core    │         │
│  │ • Idle/Walk/Attack      │  │    (C++)          │         │
│  │                         │  │                   │         │
│  │ EnemyStateManager       │  │ • Genetic Algo    │         │
│  │ • Idle/Walk/Attack/Die  │  │ • Metrics System  │         │
│  └─────────────────────────┘  │ • Parameter Mgmt  │         │
│                               └───────────────────┘         │
└─────────────────────────────────────────────────────────────┘
```

### Hybrid AI System Architecture

The framework implements a two-tier AI system:

#### Tier 1: Behavioral AI (Unity/C#)
**Finite State Machines** control immediate gameplay behaviors:

```
Enemy FSM States:
┌──────────┐     ┌──────────┐     ┌──────────┐
│   Idle   │────►│  Walk    │────►│  Attack  │
└──────────┘     └──────────┘     └──────────┘
      │                │                 │
      ▼                ▼                 ▼
┌──────────┐     ┌──────────┐     ┌──────────┐
│ Knocked  │◄────┤   Die    │◄────┤  (any)   │
└──────────┘     └──────────┘     └──────────┘

Player FSM States:
┌──────────┐     ┌──────────┐     ┌──────────┐
│   Idle   │◄───►│   Walk   │────►│  Attack  │
└──────────┘     └──────────┘     └──────────┘
                       │
                       ▼
                 ┌──────────┐
                 │  Shift   │ (Room Transition)
                 └──────────┘
```

#### Tier 2: Adaptive AI (C++ DDA Engine)
**Genetic Algorithm** evolves parameters that influence Tier 1 behaviors:
- Enemy aggressiveness, accuracy, reaction time
- Movement speed, detection range, attack frequency
- Room enemy density, powerup frequency
- Obstacle complexity, hazard intensity

### Component Interactions

```mermaid
sequenceDiagram
    participant Game
    participant DDAEngine
    participant MetricManager
    participant GeneticAlgorithm
    participant Parameters

    Game->>DDAEngine: CollectMetric(kills, 5)
    DDAEngine->>MetricManager: pushMetric("enemies_killed", 5)
    
    Game->>DDAEngine: EvolveParameters()
    DDAEngine->>MetricManager: getComputedValues()
    MetricManager-->>DDAEngine: Performance Data
    
    DDAEngine->>GeneticAlgorithm: evolve(metrics)
    GeneticAlgorithm->>GeneticAlgorithm: evaluateFitness()
    GeneticAlgorithm->>GeneticAlgorithm: selection()
    GeneticAlgorithm->>GeneticAlgorithm: crossover()
    GeneticAlgorithm->>GeneticAlgorithm: mutation()
    GeneticAlgorithm-->>DDAEngine: Best Individual
    
    DDAEngine->>Parameters: updateTargetParams()
    
    loop Every Frame
        Game->>DDAEngine: UpdateAdaptive(deltaTime)
        DDAEngine->>Parameters: smoothTransition()
        DDAEngine-->>Game: Current Parameters
    end
```

### Key Architectural Patterns

1. **Singleton Pattern**: Used for global access to MetricManager and DDAEngine
2. **Template Method Pattern**: Metric computation strategies
3. **Strategy Pattern**: Different DDA modes (Adaptive/Fixed/Learning)
4. **Observer Pattern**: Implicit in metric collection system
5. **Facade Pattern**: C API provides simplified interface to C++ internals

### Detailed Component Architecture

#### DDA Engine Core (DDAEngine.hpp/cpp)
The heart of the system is a singleton-pattern engine that:
- **Manages game difficulty parameters** through AI and PCG (Procedural Content Generation) settings
- **Operates in three modes**:
  - **Adaptive**: Smoothly transitions parameters over time using interpolation
  - **Fixed**: Maintains static parameters for consistent difficulty
  - **Learning**: Continuously evolves parameters based on ongoing performance
- **Features smooth parameter interpolation** using linear interpolation (lerp) for gradual difficulty changes
- **Tracks player performance history** maintaining up to 100 recent performance records

#### Parameter System (DDAParameters.hpp)
A hierarchical parameter structure with:
- **AIParameters**: 
  - `aggressiveness` (0.0-1.0): Enemy attack behavior intensity
  - `reactionTime` (0.1-3.0s): Enemy response delay
  - `accuracy` (0.0-1.0): Enemy hit probability
  - `movementSpeed` (0.1-3.0x): Enemy movement multiplier
  - `detectionRange` (1.0-50.0): Enemy awareness radius
  - `attackFrequency` (0.0-1.0): Attack rate modifier
- **PCGParameters**:
  - `enemyDensity` (0.0-1.0): Spawn rate modifier
  - `powerUpFrequency` (0.0-1.0): Item spawn probability
  - `obstacleComplexity` (0.0-1.0): Level complexity factor
  - `pathBranching` (0.0-1.0): Level path diversity
  - `hazardIntensity` (0.0-1.0): Environmental danger level
  - `minEnemiesPerRoom` (0-10): Minimum enemy count
  - `maxEnemiesPerRoom` (0-20): Maximum enemy count

#### Metrics System Architecture
A flexible, type-safe metrics collection framework:

**MetricManager (Singleton)**:
```cpp
std::unordered_map<std::string, std::unique_ptr<Metric>> metrics;
```
- Manages all metrics through a hash map
- Supports multiple metric types (SUM, AVERAGE, COUNT, MINIMUM, MAXIMUM, UNIQUE_COUNT, VARIANCE, STD_DEV)
- Type-safe metric storage using templates

**MetricData (Template Class)**:
```cpp
template <typename T>
struct MetricData : Metric {
    V value;
    DataStore<V> dataStore;
};
```

#### C API Integration
The C API provides 23 exported functions for complete engine control:
- Initialization and shutdown
- Metric collection (float and int variants)
- Parameter retrieval and modification
- Mode control and evolution triggers
- JSON import/export for persistence

## Demo Implementation - Unity Dungeon Crawler

### Game Overview

The demo implementation is a procedurally generated dungeon crawler that showcases the full capabilities of the adaptive AI framework. The game features:

- **100-room dungeons** arranged in a 10x10 grid
- **Real-time combat** with sword-based melee system
- **Procedural room generation** influenced by DDA parameters
- **Enemy AI** that adapts behavior based on player performance
- **Room-based progression** with smooth camera transitions

### Integration Architecture

### 1. DDAEngineWrapper - Core Integration Layer

```csharp
public class DDAEngineWrapper : MonoBehaviour
{
    // Singleton implementation for global access
    private static DDAEngineWrapper instance;
    
    // P/Invoke declarations for C API
    [DllImport("DDAEngine")]
    private static extern void DDA_Initialize();
    
    [DllImport("DDAEngine")]
    private static extern void DDA_GetAIParameters(out AIParameters outParams);
    
    // Initialization in Unity lifecycle
    void Awake()
    {
        DDA_Initialize();
        DDA_SetIdealMetrics(300f, 0.2f, 0.7f);  // 5 min levels, 20% death rate, 70% accuracy
        DDA_SetMode((int)DDAMode.Adaptive);
        DDA_SetEvolutionEnabled(1);
    }
    
    // Smooth parameter updates every frame
    void Update()
    {
        DDA_UpdateAdaptive(Time.deltaTime);
    }
}
```

### 2. Gameplay Metric Collection

The wrapper provides intuitive methods for collecting gameplay metrics:

```csharp
public class GameplayExample
{
    void OnEnemyKilled()
    {
        DDAEngineWrapper.Instance.RecordEnemyKill();
        DDAEngineWrapper.Instance.RecordShot(true);  // Hit
    }
    
    void OnPlayerDeath()
    {
        DDAEngineWrapper.Instance.RecordPlayerDeath();
    }
    
    void OnLevelComplete()
    {
        DDAEngineWrapper.Instance.EndLevel();  // Submits all metrics
    }
}
```

### 3. Dungeon Generation System

The DungeonManager creates a 100-room procedurally generated dungeon:

```csharp
public class DungeonManager : Singleton<DungeonManager>
{
    private Room[,] dungeon = new Room[10, 10];
    
    void GenerateDungeon()
    {
        // Create 10x10 grid of rooms
        for (int row = 0; row < 10; row++)
        {
            for (int col = 0; col < 10; col++)
            {
                CreateRoom(row, col);
            }
        }
        
        // Generate room connections
        GenerateDoorways();
        
        // Apply DDA parameters to room generation
        ApplyDDAParameters();
    }
}
```

### 4. Room-Based PCG with DDA Integration

Each room is procedurally generated with DDA-influenced parameters:

```csharp
public class RoomManager : MonoBehaviour
{
    void GenerateRoom()
    {
        PCGParameters pcgParams = DDAEngineWrapper.Instance.GetPCGParameters();
        
        // Generate room tiles
        GenerateTiles();
        
        // Spawn enemies based on DDA parameters
        int enemyCount = Random.Range(
            pcgParams.minEnemiesPerRoom,
            pcgParams.maxEnemiesPerRoom + 1
        );
        
        for (int i = 0; i < enemyCount; i++)
        {
            Vector2 spawnPos = GetRandomFloorPosition();
            Instantiate(enemyPrefab, spawnPos, Quaternion.identity);
        }
        
        // Apply other PCG parameters
        GenerateObstacles(pcgParams.obstacleComplexity);
        GeneratePowerUps(pcgParams.powerUpFrequency);
    }
}
```

### 5. FSM-Based Enemy AI

Enemy behavior is controlled by state machines with DDA parameter influence:

```csharp
public class EnemyStateManager : EntityStateManager
{
    private AIParameters aiParams;
    
    void Start()
    {
        // Initialize with idle state
        ChangeState(new EnemyIdleState());
        
        // Fetch AI parameters
        aiParams = DDAEngineWrapper.Instance.GetAIParameters();
    }
    
    public void HandleHit(float damage)
    {
        health -= damage;
        
        if (health <= 0)
        {
            ChangeState(new EnemyDieState());
            DDAEngineWrapper.Instance.RecordEnemyKill();
        }
        else
        {
            ChangeState(new EnemyKnockedState());
        }
    }
}

// Example state implementation
public class EnemyAttackState : IState
{
    public void Execute(EnemyStateManager enemy)
    {
        AIParameters ai = enemy.aiParams;
        
        // Attack with DDA-influenced frequency
        if (Time.time > nextAttackTime)
        {
            nextAttackTime = Time.time + (1f / ai.attackFrequency);
            
            // Apply accuracy parameter
            if (Random.value < ai.accuracy)
            {
                player.TakeDamage(damage * ai.aggressiveness);
            }
        }
    }
}
```

### 6. Player State Management

The player character uses FSM for clean state management:

```csharp
public class PlayerStateManager : EntityStateManager
{
    public void HandleInput()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            ChangeState(new PlayerSwingSwordState());
            
            // Record attack for accuracy tracking
            bool hit = CheckEnemyHit();
            DDAEngineWrapper.Instance.RecordShot(hit);
        }
    }
    
    public void TakeDamage(float damage)
    {
        health -= damage;
        DDAEngineWrapper.Instance.RecordDamage(damage);
        
        if (health <= 0)
        {
            DDAEngineWrapper.Instance.RecordPlayerDeath();
            // Handle death state
        }
    }
}
```

### Integration Benefits

1. **Hybrid AI System**: Combines immediate FSM behaviors with long-term GA adaptation
2. **Seamless Parameter Flow**: DDA parameters directly influence FSM behaviors
3. **Real-Time Adaptation**: Parameters update smoothly during gameplay
4. **Comprehensive Metrics**: Every significant game event feeds back into the DDA system
5. **Modular Architecture**: Clean separation allows independent development of AI and DDA systems

### Data Flow in Complete System

```
Game Events (Unity/C#)
    ↓
FSM State Changes → Metric Collection → DDAEngineWrapper
    ↓                                         ↓
Enemy/Player                            P/Invoke API
Behaviors                                     ↓
    ↑                                   DDA Engine (C++)
    │                                         ↓
    │                                  Genetic Algorithm
    │                                         ↓
    └──── AI/PCG Parameters ←─── Parameter Evolution
```

### Key Framework Features

1. **Behavioral AI (FSM)**:
   - Clean state-based architecture
   - Easy to extend with new states
   - Direct parameter influence on behaviors

2. **Adaptive Difficulty (GA)**:
   - Evolves 14 different parameters
   - Multi-metric fitness evaluation
   - Smooth parameter transitions

3. **Procedural Generation**:
   - Room-based dungeon system
   - DDA-influenced enemy placement
   - Dynamic obstacle and item generation

4. **Metrics Integration**:
   - Automatic collection at key events
   - JSON-based data exchange
   - Real-time performance tracking

This integration demonstrates how the DDAEngine provides a complete solution for adaptive difficulty, from metric collection through parameter evolution to real-time gameplay adjustment, all while maintaining clean separation between the game logic and the adaptation system.

## Advanced Features & Technical Details

### Performance Optimizations

1. **Static String Buffers**: The C API uses static unique_ptr strings to avoid repeated allocations
2. **Template-Based Type Efficiency**: Compile-time type resolution reduces runtime overhead
3. **Deque-Based Storage**: O(1) insertion and removal for sliding window operations
4. **Lazy Evaluation**: Metrics are only computed when requested

### Memory Management

- **Smart Pointers**: Extensive use of unique_ptr for automatic memory management
- **RAII Principles**: Resource acquisition is initialization pattern throughout
- **No Manual Memory Management**: Eliminates memory leaks and dangling pointers

### Error Handling Strategy

```cpp
try {
    MetricManager::getInstance()->pushMetric(metricName, value);
} catch (...) {
    // Silent failure to prevent game crashes
}
```

### Evolution Timing Control

- Default evolution interval: 5 minutes (configurable)
- Learning mode: Immediate evolution after each level
- Adaptive mode: Smooth transitions prevent jarring changes

### JSON Serialization Format

```json
{
  "current_parameters": {
    "ai": {
      "aggressiveness": 0.5,
      "reactionTime": 1.0,
      "accuracy": 0.7
    },
    "pcg": {
      "enemyDensity": 0.5,
      "powerUpFrequency": 0.3
    },
    "difficultyMultiplier": 1.0
  },
  "player_skill_level": 0.65,
  "fitness_score": 85.2,
  "generation_hints": {
    "level_type": "standard",
    "recommended_difficulty": 0.65
  }
}
```

## Architecture Strengths

1. **Modularity**: Clean separation between metrics, evolution, and parameters
2. **Extensibility**: Easy to add new metrics or parameter types
3. **Type Safety**: Heavy use of C++ type system for compile-time guarantees
4. **Performance**: Efficient data structures and algorithms
5. **Integration**: Well-designed C API for cross-language compatibility
6. **Real-World Ready**: Production-quality code with proper error handling

## Potential Improvements

1. **Thread Safety**: Add mutex protection for concurrent access
2. **Persistence**: Implement save/load for evolution history
3. **Analytics**: Add built-in visualization tools
4. **Network Support**: Enable multiplayer difficulty synchronization
5. **Machine Learning**: Integrate neural networks for more sophisticated adaptation
6. **Profiling**: Add performance monitoring capabilities

## Technical Achievements

### Hybrid AI Architecture
The framework successfully demonstrates how traditional AI techniques (FSM) can be enhanced with evolutionary algorithms (GA) to create a more sophisticated adaptive system. The FSM layer provides predictable, designable behaviors while the GA layer ensures these behaviors adapt to player skill over time.

### Cross-Language Integration
The seamless integration between C++ (optimization engine) and C# (gameplay systems) through a well-designed C API showcases how performance-critical algorithms can be separated from gameplay logic without sacrificing functionality or ease of use.

### Real-World Application
The dungeon crawler demo proves the framework's viability in actual game development:
- 100+ rooms with unique layouts
- Multiple enemy types with distinct behaviors
- Real-time combat with adaptive difficulty
- Smooth transitions between difficulty levels

## Conclusion

Our Adaptive Game AI Framework represents a practical approach to creating intelligent, responsive game systems. By combining Finite State Machines for behavioral control with Genetic Algorithms for parameter optimization, we've created a system that:

1. **Adapts to Players**: Continuously evolves to match player skill levels
2. **Maintains Designer Control**: FSM structure ensures predictable, tunable behaviors
3. **Scales Efficiently**: Modular architecture supports games of varying complexity
4. **Integrates Seamlessly**: Clean API design enables easy adoption in existing projects

The framework demonstrates that sophisticated adaptive AI doesn't require opaque machine learning models. Instead, by combining well-understood techniques in novel ways, we can create game AI that is both intelligent and interpretable, providing engaging experiences that grow with the player.

Whether you're building a simple action game or a complex RPG, this framework provides the tools needed to create AI that learns, adapts, and enhances the player experience—all while maintaining the performance and control that game developers require.