#include <SFML/Graphics.hpp>

#include <vector>
#include <deque>
#include <memory>
#include <cmath>
#include <algorithm>

enum class ActionStatus {
    Running,
    Finished
};

class Action {
public:
    virtual ~Action() = default;
    virtual void on_start() {}
    virtual ActionStatus update(float dt) = 0;
};

class ActionQueue {
public:
    void push(std::unique_ptr<Action> action) {
        queue.push_back(std::move(action));
    }

    void update(float dt) {
        if (queue.empty()) return;

        if (!started) {
            queue.front()->on_start();
            started = true;
        }

        if (queue.front()->update(dt) == ActionStatus::Finished) {
            queue.pop_front();
            started = false; // Next action will call on_start() on next frame
        }
    }

    bool is_empty() const { return queue.empty(); }
    void clear() { queue.clear(); started = false; }

private:
    std::deque<std::unique_ptr<Action>> queue;
    bool started = false;
};

struct AppState {
    // Transformation variables modified by our Action Queue
    float rotationAngle = 0.0f; // in radians
    float scale = 1.0f;
    sf::Color clearColor{31, 31, 38};

    ActionQueue actionQueue;
};

class RotateAction : public Action {
public:
    RotateAction(AppState* state, float targetAngleRad, float duration)
        : state(state), target(targetAngleRad), duration(duration) {}

    void on_start() override {
        startAngle = state->rotationAngle;
        elapsed = 0.0f;
    }

    ActionStatus update(float dt) override {
        elapsed += dt;
        float t = (duration > 0.0f) ? std::min(1.0f, elapsed / duration) : 1.0f;
        state->rotationAngle = startAngle + (target - startAngle) * t;
        return (t >= 1.0f) ? ActionStatus::Finished : ActionStatus::Running;
    }

private:
    AppState* state;
    float startAngle = 0.0f;
    float target;
    float duration;
    float elapsed = 0.0f;
};

class PulseAction : public Action {
public:
    PulseAction(AppState* state, float maxScale, float duration)
        : state(state), maxScale(maxScale), duration(duration) {}

    ActionStatus update(float dt) override {
        elapsed += dt;
        float t = std::min(1.0f, elapsed / duration);
        state->scale = 1.0f + (maxScale - 1.0f) * std::sin(t * 3.14159265f);
        return (t >= 1.0f) ? ActionStatus::Finished : ActionStatus::Running;
    }

private:
    AppState* state;
    float maxScale;
    float duration;
    float elapsed = 0.0f;
};

class FadeColorAction : public Action {
public:
    FadeColorAction(AppState* state, sf::Color targetColor, float duration)
        : state(state), target(targetColor), duration(duration) {}

    void on_start() override {
        start = state->clearColor;
        elapsed = 0.0f;
    }

    ActionStatus update(float dt) override {
        elapsed += dt;
        float t = (duration > 0.0f) ? std::min(1.0f, elapsed / duration) : 1.0f;

        auto lerp = [](uint8_t a, uint8_t b, float factor) -> uint8_t {
            return static_cast<uint8_t>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * factor);
        };

        state->clearColor = sf::Color(
            lerp(start.r, target.r, t),
            lerp(start.g, target.g, t),
            lerp(start.b, target.b, t)
        );

        return (t >= 1.0f) ? ActionStatus::Finished : ActionStatus::Running;
    }

private:
    AppState* state;
    sf::Color start;
    sf::Color target;
    float duration;
    float elapsed = 0.0f;
};

static void setupCutscene(AppState* state) {
    // Rotate 360 degrees (2*PI) in 1.5 seconds
    state->actionQueue.push(std::make_unique<RotateAction>(state, 6.2831853f, 1.5f));

    // Pulse scale up to 1.8x and back in 1.0 second
    state->actionQueue.push(std::make_unique<PulseAction>(state, 1.8f, 1.0f));

    // Shift background to vibrant purple over 1.0 second
    state->actionQueue.push(std::make_unique<FadeColorAction>(state, sf::Color(89, 25, 115), 1.0f));

    // Rotate back to 0 in 1.2 seconds
    state->actionQueue.push(std::make_unique<RotateAction>(state, 0.0f, 1.2f));

    // Shift background back to dark slate in 1.0 second
    state->actionQueue.push(std::make_unique<FadeColorAction>(state, sf::Color(31, 31, 38), 1.0f));
}

int main() {
    sf::RenderWindow window(sf::VideoMode({800u, 600u}), "Action Queue Cutscene Demo (SFML 3)");
    window.setFramerateLimit(60);

    AppState state;
    setupCutscene(&state);

    // Setup an RGB triangle centered around its local origin (0, 0)
    sf::VertexArray triangle(sf::PrimitiveType::Triangles, 3);
    triangle[0].position = sf::Vector2f{  0.0f, -120.0f}; // Top vertex
    triangle[0].color    = sf::Color{255, 51, 51};        // Red

    triangle[1].position = sf::Vector2f{-100.0f,   80.0f}; // Bottom-left vertex
    triangle[1].color    = sf::Color{ 51, 255, 51};       // Green

    triangle[2].position = sf::Vector2f{ 100.0f,   80.0f}; // Bottom-right vertex
    triangle[2].color    = sf::Color{ 51, 102, 255};      // Blue

    sf::Clock clock;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();

        // Advance the Action Queue
        state.actionQueue.update(dt);

        // If the sequence finished, loop it again
        if (state.actionQueue.is_empty()) {
            setupCutscene(&state);
        }

        // Render Frame
        window.clear(state.clearColor);

        // Compute 2D transformation: translate to center, apply rotation and scale
        sf::Transform transform;
        transform.translate(sf::Vector2f{400.0f, 300.0f});
        transform.rotate(sf::radians(state.rotationAngle));
        transform.scale(sf::Vector2f{state.scale, state.scale});

        window.draw(triangle, transform);
        window.display();
    }

    return 0;
}
