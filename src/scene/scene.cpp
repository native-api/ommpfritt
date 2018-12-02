#include "scene.h"
#include <random>
#include <glog/logging.h>
#include <assert.h>
#include <QDebug>

#include "objects/empty.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "properties/boolproperty.h"
#include "serializers/abstractserializer.h"
#include "commands/command.h"
#include "properties/referenceproperty.h"
#include "renderers/style.h"

namespace
{

constexpr auto ROOT_POINTER = "root";

auto make_root()
{
  return std::make_unique<omm::Empty>();
}

template<typename T> bool is_selected(const T* t) { return t->is_selected(); }

template<typename SetA, typename SetB> SetA merge(SetA&& a, SetB&& b)
{
  a.insert(b.begin(), b.end());
  return a;
}

template<typename SetA, typename SetB, typename... Sets>
SetA merge(SetA&& a, SetB&& b, Sets&&... sets)
{
  return merge(merge(a, b), std::forward<Sets>(sets)...);
}

std::unique_ptr<omm::Style> make_default_style()
{
  auto default_style = std::make_unique<omm::Style>();
  default_style->property<omm::BoolProperty>(omm::Style::PEN_IS_ACTIVE_KEY).set_value(true);
  default_style->property<omm::BoolProperty>(omm::Style::BRUSH_IS_ACTIVE_KEY).set_value(true);
  return default_style;
}

}  // namespace

namespace omm
{

Scene* Scene::m_current = nullptr;

Scene::Scene()
  : m_root(make_root())
  , m_default_style(make_default_style())
{
  m_root->property<StringProperty>(Object::NAME_PROPERTY_KEY).value() = "_root_";
  m_current = this;
}

Scene::~Scene()
{
  if (m_current == this) {
    m_current = nullptr;
  }
}

ObjectView Scene::root_view()
{
  return ObjectView(*m_root);
}

Scene* Scene::currentInstance()
{
  return m_current;
}

void Scene::insert_object(std::unique_ptr<Object> object, Object& parent)
{
  size_t n = parent.children().size();

  Observed<AbstractObjectTreeObserver>::for_each(
    [&parent, n] (auto* observer) { observer->beginInsertObject(parent, n); }
  );

  parent.adopt(std::move(object));

  Observed<AbstractObjectTreeObserver>::for_each(
    [] (auto* observer) { observer->endInsertObject(); }
  );

  objects.invalidate();
  tags.invalidate();
}

Object& Scene::root() const
{
  return *m_root;
}

std::set<ReferenceProperty*>
Scene::find_reference_holders(const AbstractPropertyOwner& candidate) const
{
  std::set<ReferenceProperty*> reference_holders;
  for (const auto& property_owner : property_owners()) {
    const auto& property_map = property_owner->properties();
    for (const auto& key : property_map.keys()) {
      auto* reference_property = property_map.at(key)->cast<ReferenceProperty>();
      if (reference_property != nullptr && reference_property->value() == &candidate) {
        reference_holders.insert(reference_property);
      }
    }
  }
  return reference_holders;
}

void Scene::selection_changed()
{
  const auto selected_objects = selection();

  Observed<AbstractSelectionObserver>::for_each(
    [selected_objects](auto* observer) { observer->set_selection(selected_objects); }
  );
}

void Scene::clear_selection()
{
  for (auto& o : root().all_descendants()) {
    o->set_selected(false);
    for (auto& t : o->tags()) {
      t->set_selected(false);
    }
  }
  selection_changed();
}

void Scene::move_object(MoveObjectTreeContext context)
{
  assert(context.is_valid());
  Object& old_parent = context.subject.get().parent();

  Observed<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginMoveObject(context); }
  );
  context.parent.get().adopt(old_parent.repudiate(context.subject), context.predecessor);
  Observed<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endMoveObject(); }
  );

  objects.invalidate();
  tags.invalidate();
}

void Scene::insert_object(OwningObjectTreeContext& context)
{
  assert(context.subject.owns());
  Observed<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginInsertObject(context); }
  );
  context.parent.get().adopt(context.subject.release(), context.predecessor);
  Observed<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endInsertObject(); }
  );

  objects.invalidate();
  tags.invalidate();
}

void Scene::remove_object(OwningObjectTreeContext& context)
{
  assert(!context.subject.owns());
  Observed<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginRemoveObject(context.subject); }
  );
  context.subject.capture(context.parent.get().repudiate(context.subject));
  Observed<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endRemoveObject(); }
  );

  objects.invalidate();
  tags.invalidate();
}

bool Scene::save_as(const std::string &filename)
{
  std::ofstream ofstream(filename);
  if (ofstream) {
    auto serializer = AbstractSerializer::make( "JSONSerializer",
                                                static_cast<std::ostream&>(ofstream) );
    root().serialize(*serializer, ROOT_POINTER);

    LOG(INFO) << "Saved current scene to '" << filename << "'";
    set_has_pending_changes(false);
    m_filename = filename;
    return true;
  } else {
    LOG(ERROR) << "Failed to open ofstream at '" << filename << "'";
    return false;
  }
}

bool Scene::load_from(const std::string &filename)
{
  std::ifstream ifstream(filename);
  if (ifstream) {
    auto deserializer = AbstractDeserializer::make( "JSONDeserializer",
                                                    static_cast<std::istream&>(ifstream) );
    try {
      auto new_root = make_root();
      new_root->deserialize(*deserializer, ROOT_POINTER);
      replace_root(std::move(new_root));
    } catch (const AbstractDeserializer::DeserializeError& deserialize_error) {
      LOG(ERROR) << "Failed to deserialize file at '" << filename << "'.";
      LOG(INFO) << deserialize_error.what();
      return false;
    }

    set_has_pending_changes(false);
    m_filename = filename;
    return true;
  } else {
    LOG(ERROR) << "Failed to open '" << filename << "'.";
    return false;
  }
}

void Scene::reset()
{
  set_has_pending_changes(false);
  replace_root(make_root());
}

std::string Scene::filename() const
{
  return m_filename;
}

void Scene::set_has_pending_changes(bool v)
{
  m_has_pending_changes = v;
}

bool Scene::has_pending_changes() const
{
  return m_has_pending_changes;
}

void Scene::submit(std::unique_ptr<Command> command)
{
  m_undo_stack.push(command.release());
  set_has_pending_changes(true);
}

QUndoStack& Scene::undo_stack()
{
  return m_undo_stack;
}

std::unique_ptr<Object> Scene::replace_root(std::unique_ptr<Object> new_root)
{
  auto old_root = std::move(m_root);
  m_root = std::move(new_root);
  return old_root;
}

template<> std::set<Object*> Scene::TGetter<Object>::compute() const
{
  return m_self.root().all_descendants();
}

template<> std::set<Tag*> Scene::TGetter<Tag>::compute() const
{
  std::set<Tag*> tags;
  for (const auto object : m_self.objects()) {
    tags = merge(tags, object->tags());
  }
  return tags;
}

std::set<Style*> Scene::styles() const
{
  return ::transform<Style*, std::set>(m_styles, [](auto&& style) { return style.get(); });
}

Style* Scene::style(size_t i) const
{
  return m_styles[i].get();
}

std::set<Object*> Scene::selected_objects() const
{
  return ::filter_if(objects(), is_selected<Object>);
}

std::set<Tag*> Scene::selected_tags() const
{
  return ::filter_if(tags(), is_selected<Tag>);
}

std::set<Style*> Scene::selected_styles() const
{
  return ::filter_if(styles(), is_selected<Style>);
}

std::set<AbstractPropertyOwner*> Scene::property_owners() const
{
  return merge(std::set<AbstractPropertyOwner*>(), objects(), tags(), styles());
}

std::set<AbstractPropertyOwner*> Scene::selection() const
{
  return merge( std::set<AbstractPropertyOwner*>(),
                selected_objects(), selected_tags(), selected_styles());
}

Tag& Scene::attach_tag(Object& owner, std::unique_ptr<Tag> tag)
{
  const auto n = owner.n_tags();
  const Tag* predecessor = n == 0 ? nullptr : &owner.tag(n-1);
  return attach_tag(owner, std::move(tag), predecessor);
}

Tag& Scene::attach_tag(Object& owner, std::unique_ptr<Tag> tag, const Tag* predecessor)
{
  Tag& ref = owner.attach_tag(std::move(tag), predecessor);
  tags.invalidate();
  selection_changed();
  return ref;
}

std::unique_ptr<Tag> Scene::detach_tag(Object& owner, Tag& tag)
{
  auto ref = owner.detach_tag(tag);
  tags.invalidate();
  selection_changed();
  return ref;
}

Style& Scene::default_style() const
{
  return *m_default_style;
}

void Scene::insert_style(std::unique_ptr<Style> style)
{
  Observed<AbstractStyleListObserver>::for_each([this](auto* observer){
    observer->beginInsertObject(m_styles.size());
  });
  m_styles.push_back(std::move(style));
  Observed<AbstractStyleListObserver>::for_each([](auto* observer){
    observer->endInsertObject();
  });
  selection_changed();
}

std::unique_ptr<Style> Scene::remove_style(Style& style)
{
  auto ref = ::extract(m_styles, style);
  selection_changed();
  return ref;
}

}  // namespace omm
