#pragma once
#include "raylib.h"
#include "box2d/box2d.h"
#include "box2d/base.h"
#include <LDtkLoader/Project.hpp>
#include <LDtkLoader/World.hpp>
#include <vector>
#include <optional>
#include <list>
#include "car.h"


namespace scene
{
    struct Entity
    {
        std::optional<b2BodyId> bodyId;
        b2Vec2 extent;
        Vector2 pos;
    };

    struct Joint {
        Entity* nodeA;
        Entity* nodeB;
        b2JointId id;
    };

    enum class LevelState {
        PLAYING,
        PASSED,
        LOSE
    };

    class SceneManager
    {
    public:
        static SceneManager* getInstance();
        static void cleanup();
        void Load();
        void Update();
        void Draw();
        void Reset();
        Rectangle& ScreenInWorld();
        bool IsLevelClear();
        void NextLevel();
        bool IsLastLevel();
        void setLevel(int level);
        void MoveCar();
        void SetTutotrialPassed();
    private:
        SceneManager();
        ~SceneManager() = default;
        void createB2World();
        void checkCollisions();
        void checkNodesCollision();
        void checkCarCollision();
        bool checkEntityCollision(Entity* entity, Vector2 point);
        void DrawEntity(const Entity& entity, Color color);
        void DrawJointBodies(const Entity& entity, Color color);
        void DrawJoint(const Joint& joint);
        void AddJoint(Entity* entityA, Entity* entityB);
        void addNode(Vector2 position);
        inline static SceneManager* instance = nullptr;
        LevelState state = LevelState::PLAYING;
        ldtk::Project ldtkProject;
        const ldtk::World* ldtkWorld{};
        const ldtk::Level* currentLdtkLevel{};
        int currentLevel = 0;
        int maxLevels = 1;
        Texture2D currentTilesetTexture;
        Texture2D renderedLevelTexture;
        Rectangle screenInWorld;
        Camera2D worldCamera = { };
        float seconds = {};
        std::optional<b2WorldId> worldId;
        std::vector<Entity> groundEntities;
        std::list<Entity> nodeEntities;
        std::vector<Joint> jointEntities;
        std::vector<Entity> jointBodyEntities;
        Entity passedEntity;
        Entity loseEntity;
        Entity* focusNode = nullptr;
        Entity* selectedNode = nullptr;
        Vector2 mousePosition;
        Car m_car;
        int tutorialStep = 0;
        std::vector<Texture2D> tutorials;
        std::vector<Vector2> tutorialPos;
        bool tutorialPassed = false;
    };

}