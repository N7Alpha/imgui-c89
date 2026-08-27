#define IMGUI_FIXTURE_DEFAULT_SCALE 1.0f

int GlobalDouble(int value)
{
    return value * 2;
}

namespace ImGui
{
int DestructorState;

struct Tracked
{
    int value;
    Tracked();
    Tracked(int value_arg);
    ~Tracked();
};

Tracked::Tracked() : value(0) {}
Tracked::Tracked(int value_arg) : value(value_arg) {}
Tracked::~Tracked() { DestructorState = DestructorState * 10 + value; }

struct ZOwner
{
    Tracked member;
    ZOwner(int value);
    ~ZOwner();
};

ZOwner::ZOwner(int value) : member(value) {}
ZOwner::~ZOwner() { DestructorState = DestructorState * 10 + 9; }

int CleanupOrder()
{
    DestructorState = 0;
    {
        ZOwner owner(1);
        Tracked later(2);
    }
    return DestructorState;
}

int ReturnCleanup()
{
    Tracked local(7);
    DestructorState = 0;
    return DestructorState;
}

int ReadDestructorState()
{
    return DestructorState;
}

int LoopCleanup()
{
    int index;
    DestructorState = 0;
    for (index = 0; index < 3; ++index)
    {
        Tracked local(index + 1);
        if (!index)
            continue;
        break;
    }
    return DestructorState;
}

int StackScratch(int count)
{
    int index;
    int result = 0;
    int* values = (int*)__builtin_alloca(sizeof(int) * count);
    for (index = 0; index < count; ++index)
    {
        values[index] = index + 1;
        result += values[index];
    }
    return result;
}
}

namespace ImGui
{
struct Vec2
{
    float x;
    float y;
    Vec2();
    Vec2(float x_value, float y_value);
    float Sum() const;
};

Vec2::Vec2() : x(0.0f), y(0.0f) {}
Vec2::Vec2(float x_value, float y_value) : x(x_value), y(y_value) {}
float Vec2::Sum() const { return x + y; }

static Vec2& operator+=(Vec2& self, float value)
{
    self.x += value;
    return self;
}

float DiscardedReference()
{
    Vec2 value(2.0f, 3.0f);
    value += 4.0f;
    return value.Sum();
}

float ScaledSum(const Vec2& value, float scale = IMGUI_FIXTURE_DEFAULT_SCALE)
{
    return value.Sum() * scale;
}

int SumTo(int count)
{
    int result = 0;
    int index;
    for (index = 0; index < count; ++index)
        result += index;
    return result;
}

int SwitchValue(int value)
{
    switch (value)
    {
    case 1:
        return 2;
    default:
        return 3;
    }
}

int GotoValue(int value)
{
    if (value < 0)
        goto negative;
    return value;
negative:
    return -value;
}
}
