#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "TextureManager.h"
#include "Costume.h"
#include "FntFont.h"
#include "Entity.h"
#include "Lip.h"
#include "Object.h"
#include "SoundDefinition.h"
#include "InventoryObject.h"

namespace ng
{
class Room;
class Engine;

class Actor : public Entity
{
private:
  class WalkingState
  {
  public:
    WalkingState(Actor &actor);

    void setDestination(const sf::Vector2f &destination, Facing facing);
    void update(const sf::Time &elapsed);
    void stop();
    bool isWalking() const { return _isWalking; }

  private:
    Actor &_actor;
    sf::Vector2f _destination;
    Facing _facing;
    bool _isWalking;
  };

  class TalkingState : public sf::Drawable
  {
  public:
    TalkingState(Actor &actor);

    void setTalkOffset(const sf::Vector2i &offset) { _talkOffset = offset; }
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    void update(const sf::Time &elapsed);
    void say(int id);
    void stop();
    bool isTalking() const { return _isTalking; }
    void setTalkColor(sf::Color color) { _talkColor = color; }

  private:
    void load(int id);

  private:
    Actor &_actor;
    FntFont _font;
    bool _isTalking;
    std::string _sayText;
    Lip _lip;
    int _index;
    sf::Vector2i _talkOffset;
    sf::Color _talkColor;
    sf::Clock _clock;
    std::vector<int> _ids;
    std::shared_ptr<SoundId> _sound;
  };

public:
  explicit Actor(Engine &engine);
  virtual ~Actor();

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }
  void setIcon(const std::string &icon) { _icon = icon; }
  const std::string &getIcon() const { return _icon; }

  void setVisible(bool isVisible) { _isVisible = isVisible; }
  bool isVisible() const { return _isVisible; }

  void useWalkboxes(bool use) { _use = use; }

  int getZOrder() const override;

  void setCostume(const std::string &name, const std::string &sheet = "");
  Costume &getCostume() { return _costume; }
  const Costume &getCostume() const { return _costume; }

  void setTalkColor(sf::Color color) { _talkingState.setTalkColor(color); }
  void setTalkOffset(const sf::Vector2i &offset) { _talkingState.setTalkOffset(offset); }
  void say(int id) { _talkingState.say(id); }
  void stopTalking() { _talkingState.stop(); }
  bool isTalking() const { return _talkingState.isTalking(); }

  void setColor(sf::Color color) { _color = color; }
  sf::Color getColor() { return _color; }

  void move(const sf::Vector2f &offset);
  void setRenderOffset(const sf::Vector2i &offset) { _renderOffset = offset; }

  Room *getRoom() const { return _pRoom; }
  void setRoom(Room *pRoom);

  void setHotspot(const sf::IntRect &hotspot) { _hotspot = hotspot; }
  const sf::IntRect &getHotspot() const { return _hotspot; }

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void update(const sf::Time &time) override;

  void pickupObject(std::unique_ptr<InventoryObject> pObject) { _objects.push_back(std::move(pObject)); }
  const std::vector<std::unique_ptr<InventoryObject>> &getObjects() const { return _objects; }

  void setWalkSpeed(const sf::Vector2i &speed) { _speed = speed; }
  const sf::Vector2i &getWalkSpeed() const { return _speed; }
  void walkTo(const sf::Vector2f &destination);
  void walkTo(const sf::Vector2f &destination, Facing facing);
  void stopWalking() { _walkingState.stop(); }
  bool isWalking() const { return _walkingState.isWalking(); }

  void setVolume(float volume) { _volume = volume; }
  float getVolume() const { return _volume; }

  void trigSound(const std::string& name) override;

private:
  Engine &_engine;
  const EngineSettings &_settings;
  Costume _costume;
  std::string _name, _icon;
  sf::Color _color;
  sf::Vector2i _renderOffset;
  int _zorder;
  bool _isVisible;
  bool _use;
  Room *_pRoom;
  sf::IntRect _hotspot;
  std::vector<std::unique_ptr<InventoryObject>> _objects;
  WalkingState _walkingState;
  TalkingState _talkingState;
  sf::Vector2i _speed;
  float _volume;
};
} // namespace ng
