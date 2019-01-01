#pragma once
#include "squirrel3/squirrel.h"
#include "Engine.h"
#include "../_NGUtil.h"

namespace ng
{
class _DefaultScriptExecute : public ScriptExecute
{
  public:
    _DefaultScriptExecute(HSQUIRRELVM vm)
        : _vm(vm)
    {
    }

  public:
    void execute(const std::string &code) override
    {
        std::string c;
        c.append("return ");
        c.append(code);
        _pos = 0;
        // compile
        sq_pushroottable(_vm);
        if (SQ_FAILED(sq_compile(_vm, program_reader, (SQUserPointer)c.data(), _SC("_DefaultScriptExecute"), SQTrue)))
        {
            std::cerr << "Error executing code " << code << std::endl;
            return;
        }
        // call
        sq_pushroottable(_vm);
        if (SQ_FAILED(sq_call(_vm, 1, SQTrue, SQTrue)))
        {
            std::cerr << "Error calling code " << code << std::endl;
            return;
        }
    }

    bool executeCondition(const std::string &code) override
    {
        execute(code);
        // get the result
        auto type = sq_gettype(_vm, -1);
        SQBool result;
        if (SQ_FAILED(sq_getbool(_vm, -1, &result)))
        {
            std::cerr << "Error getting result " << code << std::endl;
            return false;
        }
        std::cout << code << " returns " << result << std::endl;
        return result == SQTrue;
    }

    SoundDefinition *getSoundDefinition(const std::string &name) override
    {
        sq_pushroottable(_vm);
        sq_pushstring(_vm, name.data(), -1);
        sq_get(_vm, -2);
        HSQOBJECT obj;
        sq_getstackobj(_vm, -1, &obj);
        
        if (!sq_isuserpointer(obj))
        {
            std::cerr << "getSoundDefinition: sound should be a userpointer" << std::endl;
            return nullptr;
        }
        
        SoundDefinition *pSound = static_cast<SoundDefinition*>(obj._unVal.pUserPointer);
        return pSound;
    }

    static SQInteger program_reader(SQUserPointer p)
    {
        auto code = (char *)p;
        return (SQInteger)code[_pos++];
    }

  private:
    static int _pos;
    HSQUIRRELVM _vm;
};

int _DefaultScriptExecute::_pos = 0;

class _DefaultVerbExecute : public VerbExecute
{
  public:
    _DefaultVerbExecute(HSQUIRRELVM vm, Engine &engine)
        : _vm(vm), _engine(engine)
    {
    }

  private:
    void execute(const Object *pObject, const Verb *pVerb) override
    {
        auto pRoom = pObject->getRoom();
        sq_pushroottable(_vm);
        sq_pushstring(_vm, pRoom->getId().data(), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            HSQOBJECT room;
            sq_getstackobj(_vm, -1, &room);

            sq_pushobject(_vm, room);
            sq_pushstring(_vm, pObject->getId().data(), -1);

            if (SQ_SUCCEEDED(sq_get(_vm, -2)))
            {
                HSQOBJECT obj;
                sq_getstackobj(_vm, -1, &obj);

                std::string verb;
                if (pVerb)
                {
                    verb = pVerb->id;
                }
                else
                {
                    sq_pushobject(_vm, obj);
                    sq_pushstring(_vm, _SC("defaultVerb"), -1);

                    const SQChar *defaultVerb = nullptr;
                    if (SQ_SUCCEEDED(sq_get(_vm, -2)))
                    {
                        sq_getstring(_vm, -1, &defaultVerb);
                        verb = defaultVerb;
                        std::cout << "defaultVerb: " << defaultVerb << std::endl;
                    }
                    if (!defaultVerb)
                        return;
                }

                // first check for objectPreWalk
                sq_pushobject(_vm, obj);
                sq_pushstring(_vm, _SC("objectPreWalk"), -1);
                if (SQ_SUCCEEDED(sq_get(_vm, -2)))
                {
                    sq_remove(_vm, -2);
                    sq_pushobject(_vm, obj);
                    sq_pushstring(_vm, verb.data(), -1);
                    sq_pushnull(_vm);
                    sq_pushnull(_vm);
                    sq_call(_vm, 4, SQTrue, SQTrue);
                    SQInteger handled = 0;
                    sq_getinteger(_vm, -1, &handled);
                    if (handled == 1)
                        return;
                }

                pVerb = _engine.getVerb(verb);
                if (!pVerb)
                    return;
                auto func = pVerb->func;

                sq_pop(_vm, 2); //pops the roottable and the function

                sq_pushobject(_vm, obj);
                sq_pushstring(_vm, func.data(), -1);

                if (SQ_SUCCEEDED(sq_get(_vm, -2)))
                {
                    sq_remove(_vm, -2);
                    sq_pushobject(_vm, obj);
                    sq_call(_vm, 1, SQFalse, SQTrue);
                    sq_pop(_vm, 2); //pops the roottable and the function
                }
                else if (pVerb->id == "walkto")
                {
                    auto pos = pObject->getPosition();
                    auto usePos = pObject->getUsePosition();
                    auto dest = sf::Vector2f(pos.x + usePos.x, pos.y - usePos.y);
                    _engine.getCurrentActor()->walkTo(dest, _toFacing(pObject->getUseDirection()));
                }
                else
                {
                    sq_pushobject(_vm, obj);
                    sq_pushstring(_vm, _SC("verbDefault"), -1);

                    if (SQ_SUCCEEDED(sq_get(_vm, -2)))
                    {
                        sq_remove(_vm, -2);
                        sq_pushobject(_vm, obj);
                        sq_call(_vm, 1, SQFalse, SQTrue);
                        sq_pop(_vm, 2); //pops the roottable and the function
                    }
                }
            }
        }
    }

    void use(const InventoryObject *pObjectSource, const Object *pObjectTarget) override
    {
        HSQOBJECT objSource = *(HSQOBJECT *)pObjectSource->getHandle();
        HSQOBJECT objTarget = *(HSQOBJECT *)pObjectTarget->getTable();

        auto pTable = _engine.getRoom().getTable();
        sq_pushobject(_vm, objSource);
        sq_pushobject(_vm, *pTable);
        sq_setdelegate(_vm, -2);

        sq_pushobject(_vm, objSource);
        sq_pushstring(_vm, _SC("verbUse"), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, objSource);
            sq_pushobject(_vm, objTarget);
            sq_call(_vm, 2, SQFalse, SQTrue);
            sq_pop(_vm, 2); //pops the roottable and the function
            return;
        }
    }

    void execute(const InventoryObject *pObject, const Verb *pVerb) override
    {
        HSQOBJECT obj = *(HSQOBJECT *)pObject->getHandle();

        std::string verb;
        if (pVerb)
        {
            verb = pVerb->id;
        }
        else
        {
            const SQChar *defaultVerb = getDefaultVerb(obj);
            if (!defaultVerb)
                return;
            verb = defaultVerb;
        }

        // first check for objectPreWalk
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("objectPreWalk"), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, obj);
            sq_pushstring(_vm, verb.data(), -1);
            sq_pushnull(_vm);
            sq_pushnull(_vm);
            sq_call(_vm, 4, SQTrue, SQTrue);
            SQInteger handled = 0;
            sq_getinteger(_vm, -1, &handled);
            if (handled == 1)
                return;
        }
        sq_pop(_vm, 2); //pops the roottable and the function

        pVerb = _engine.getVerb(verb);
        if (!pVerb)
            return;

        SQInteger flags = 0;
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("flags"), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_getinteger(_vm, -1, &flags);
            UseFlag useFlag;
            switch (flags)
            {
            case 2:
                useFlag = UseFlag::UseWith;
                break;
            case 4:
                useFlag = UseFlag::UseOn;
                break;
            case 8:
                useFlag = UseFlag::UseIn;
                break;
            }
            _engine.setUseFlag(useFlag, pObject);
            return;
        }

        auto pTable = _engine.getRoom().getTable();
        sq_pushobject(_vm, obj);
        sq_pushobject(_vm, *pTable);
        sq_setdelegate(_vm, -2);

        auto func = pVerb->func;
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, func.data(), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, obj);
            sq_call(_vm, 1, SQFalse, SQTrue);
            sq_pop(_vm, 2); //pops the roottable and the function
            return;
        }
        callVerbDefault(obj);
    }

    void callVerbDefault(HSQOBJECT obj)
    {
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("verbDefault"), -1);

        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, obj);
            sq_call(_vm, 1, SQFalse, SQTrue);
            sq_pop(_vm, 2); //pops the roottable and the function
        }
    }

    const SQChar *getDefaultVerb(HSQOBJECT obj)
    {
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("defaultVerb"), -1);

        const SQChar *defaultVerb = nullptr;
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_getstring(_vm, -1, &defaultVerb);
            std::cout << "defaultVerb: " << defaultVerb << std::endl;
        }
        return defaultVerb;
    }

  private:
    HSQUIRRELVM _vm;
    Engine &_engine;
}; // namespace ng
} // namespace ng
