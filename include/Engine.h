#pragma once
#include "SFML/Audio.hpp"
#include "Function.h"
#include "Object.h"
#include "EngineSettings.h"
#include "Room.h"
#include "Actor.h"
#include "TextureManager.h"
#include "TextDatabase.h"
#include "Font.h"
#include "SoundDefinition.h"
#include "Verb.h"
#include "SpriteSheet.h"
#include "NonCopyable.h"
#include "Dialog/DialogManager.h"
#include "Preferences.h"

namespace ng
{
class VerbExecute
{
public:
  virtual ~VerbExecute() = default;
  virtual void execute(Object *pObject, const Verb *pVerb) = 0;
};

class ScriptExecute
{
public:
  virtual ~ScriptExecute() = default;
  virtual void execute(const std::string &code) = 0;
  virtual bool executeCondition(const std::string &code) = 0;
};

class Engine : public NonCopyable
{
public:
  explicit Engine(const EngineSettings &settings);
  ~Engine();

  void setCameraAt(const sf::Vector2f &at);
  void moveCamera(const sf::Vector2f &offset);
  sf::Vector2f getCameraAt() const { return _cameraPos; }

  void setWindow(sf::RenderWindow &window) { _pWindow = &window; }

  TextureManager &getTextureManager() { return _textureManager; }
  const EngineSettings &getSettings() const { return _settings; }

  Room &getRoom() { return *_pRoom; }
  void setRoom(Room *room) { _pRoom = room; }
  Font &getFont() { return _font; }
  std::string getText(int id) { return _textDb.getText(id); }
  void setFadeAlpha(float fade) { _fadeAlpha = static_cast<uint8_t>(fade * 255); }
  float getFadeAlpha() const { return _fadeAlpha / 255.f; }

  void addActor(std::unique_ptr<Actor> actor) { _actors.push_back(std::move(actor)); }
  void addRoom(std::unique_ptr<Room> room) { _rooms.push_back(std::move(room)); }
  void addFunction(std::unique_ptr<Function> function) { _newFunctions.push_back(std::move(function)); }

  std::vector<std::unique_ptr<Actor>> &getActors() { return _actors; }

  std::shared_ptr<SoundDefinition> defineSound(const std::string &name);
  std::shared_ptr<SoundId> playSound(SoundDefinition &soundDefinition, bool loop = false);
  std::shared_ptr<SoundId> loopMusic(SoundDefinition &soundDefinition);
  void stopSound(SoundId &sound);

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window) const;

  void setCurrentActor(Actor *pCurrentActor) { _pCurrentActor = pCurrentActor; }
  Actor *getCurrentActor() { return _pCurrentActor; }

  void setVerb(int characterSlot, int verbSlot, const Verb &verb) { _verbSlots[characterSlot].setVerb(verbSlot, verb); }
  void setVerbUiColors(int characterSlot, VerbUiColors colors) { _verbUiColors[characterSlot] = colors; }
  VerbUiColors &getVerbUiColors(int characterSlot) { return _verbUiColors[characterSlot]; }

  void setInputActive(bool active)
  {
    _inputActive = active;
    _showCursor = active;
  }
  void inputSilentOff() { _inputActive = false; }
  bool getInputActive() const { return _inputActive; }

  void follow(Actor *pActor) { _pFollowActor = pActor; }
  void setVerbExecute(std::unique_ptr<VerbExecute> verbExecute) { _pVerbExecute = std::move(verbExecute); }
  void setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute) { _pScriptExecute = std::move(scriptExecute); }
  const Verb *getVerb(const std::string &id) const;

  void addThread(HSQUIRRELVM thread) { _threads.push_back(thread); }
  void stopThread(HSQUIRRELVM thread);
  bool isThreadAlive(HSQUIRRELVM thread) const;

  void startDialog(const std::string &dialog);
  void execute(const std::string &code);
  bool executeCondition(const std::string &code);

  sf::Vector2f getMousePos() const { return _mousePos; }

  Preferences &getPreferences() { return _preferences; }

private:
  sf::IntRect getVerbRect(const std::string &name, std::string lang = "en", bool isRetro = false) const;
  void drawVerbs(sf::RenderWindow &window) const;
  void drawInventory(sf::RenderWindow &window) const;
  void drawCursor(sf::RenderWindow &window) const;

private:
  const EngineSettings &_settings;
  TextureManager _textureManager;
  Room *_pRoom;
  std::vector<std::unique_ptr<Actor>> _actors;
  std::vector<std::unique_ptr<Room>> _rooms;
  std::vector<std::unique_ptr<Function>> _newFunctions;
  std::vector<std::unique_ptr<Function>> _functions;
  std::vector<std::shared_ptr<SoundDefinition>> _sounds;
  std::vector<std::shared_ptr<SoundId>> _soundIds;
  sf::Music _music;
  sf::Uint8 _fadeAlpha;
  sf::RenderWindow *_pWindow;
  sf::Vector2f _cameraPos;
  TextDatabase _textDb;
  Font _font;
  Actor *_pCurrentActor;
  std::array<VerbSlot, 6> _verbSlots;
  std::array<VerbUiColors, 6> _verbUiColors;
  bool _inputActive;
  bool _showCursor;
  SpriteSheet _verbSheet, _gameSheet, _inventoryItems;
  nlohmann::json _jsonInventoryItems;
  Actor *_pFollowActor;
  sf::IntRect _cursorRect;
  sf::IntRect _cursorLeftRect;
  sf::IntRect _cursorRightRect;
  sf::IntRect _cursorFrontRect;
  sf::IntRect _cursorBackRect;
  sf::IntRect _hotspotCursorRect;
  sf::IntRect _hotspotCursorLeftRect;
  sf::IntRect _hotspotCursorRightRect;
  sf::IntRect _hotspotCursorFrontRect;
  sf::IntRect _hotspotCursorBackRect;
  sf::IntRect _verbRects[9];
  Object *_pCurrentObject;
  sf::Vector2f _mousePos;
  std::unique_ptr<VerbExecute> _pVerbExecute;
  std::unique_ptr<ScriptExecute> _pScriptExecute;
  const Verb *_pVerb;
  std::vector<HSQUIRRELVM> _threads;
  DialogManager _dialogManager;
  Preferences _preferences;
};
} // namespace ng