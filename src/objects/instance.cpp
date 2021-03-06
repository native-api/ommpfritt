#include "objects/instance.h"

#include "objects/empty.h"
#include <QObject>
#include "scene/scene.h"
#include "properties/referenceproperty.h"
#include "properties/boolproperty.h"
#include "commands/propertycommand.h"
#include "tags/tag.h"
#include "tags/scripttag.h"
#include "tags/styletag.h"
#include "scene/mailbox.h"

namespace omm
{

class Style;

Instance::Instance(Scene* scene)
  : Object(scene)
{
  static const auto category = QObject::tr("Instance");
  create_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_filter(ReferenceProperty::Filter({ Kind::Object }, {{}}))
    .set_label(QObject::tr("reference", "Instance")).set_category(category);
  create_property<BoolProperty>(IDENTICAL_PROPERTY_KEY)
    .set_label(QObject::tr("identical", "Instance")).set_category(category);
  polish();
}

Instance::Instance(const Instance &other) : Object(other)
{
  polish();
}

void Instance::polish()
{
  listen_to_changes([this]() {
    return kind_cast<Object*>(property(REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>());
  });

  connect(&scene()->mail_box(), &MailBox::tag_inserted, this, [this](Object& owner, Tag&) {
    if (&owner == this) {
      update();
    }
  });

  connect(&scene()->mail_box(), &MailBox::tag_removed, this, [this](Object& owner, Tag&) {
    if (&owner == this) {
      update();
    }
  });
}

void Instance::draw_object(Painter &renderer, const Style& style, Painter::Options options) const
{
  Q_UNUSED(style)
  auto cycle_guard = scene()->make_cycle_guard(this);
  if (!cycle_guard->inside_cycle() && is_active()) {
    auto* r = illustrated_object();
    if (r != nullptr) {
      r->set_virtual_parent(this);
      r->draw_recursive(renderer, options);
      r->set_virtual_parent(nullptr);
    }
  }
}

BoundingBox Instance::bounding_box(const ObjectTransformation &transformation) const
{
  auto cycle_guard = scene()->make_cycle_guard(this);
  if (!cycle_guard->inside_cycle() && is_active()) {
    const auto* r = illustrated_object();
    if (r != nullptr) {
      return r->recursive_bounding_box(transformation);
    } else {
      return BoundingBox();
    }
  } else {
    return BoundingBox();
  }
}

Object *Instance::illustrated_object() const
{
  // Note: If you implement a cache, keep in mind that it becomes dirty if
  //  - this instance's parent changes
  //  - the referenced object changes

  if (property(IDENTICAL_PROPERTY_KEY)->value<bool>()) {
    return referenced_object();
  } else {
    return m_reference.get();
  }
}

Object::ConvertedObject Instance::convert() const
{
  std::unique_ptr<Object> clone;
  if (is_active()) {
    const auto* reference = referenced_object();
    if (reference != nullptr) {
      clone = reference->clone();
      copy_properties(*clone, CopiedProperties::Compatible | CopiedProperties::User);
      copy_tags(*clone);
    }
  }
  if (!clone) {
    clone = std::make_unique<Empty>(scene());
  }
  return {std::move(clone), true};
}

QString Instance::type() const { return TYPE; }
Flag Instance::flags() const { return Object::flags() | Flag::Convertible; }

void Instance::post_create_hook()
{
  const auto selection = scene()->item_selection<Object>();
  if (selection.size() == 1) {
    auto* object = *selection.begin();
    using command = PropertiesCommand<ReferenceProperty>;
    scene()->submit<command>(std::set { property(REFERENCE_PROPERTY_KEY) }, object);
  }
}

void Instance::update()
{
  auto cycle_guard = scene()->make_cycle_guard(this);
  if (cycle_guard->inside_cycle()) {
    return;
  }
  if (is_active()) {
    m_reference.reset();
    if (!property(IDENTICAL_PROPERTY_KEY)->value<bool>()) {
      Object* r = referenced_object();
      if (r != nullptr) {
        m_reference = r->clone();
        m_reference->set_transformation(ObjectTransformation());
        m_reference->property(Object::VISIBILITY_PROPERTY_KEY)->set(Object::Visibility::Default);
        m_reference->property(Object::VIEWPORT_VISIBILITY_PROPERTY_KEY)->set(Object::Visibility::Default);
      }
    }

    update_tags();
  }
  Object::update();
}

Geom::PathVector Instance::paths() const
{
  if (m_reference) {
    return m_reference->geom_paths();
  } else {
    return Geom::PathVector();
  }
}

void Instance::on_property_value_changed(Property *property)
{
  if (    property == this->property(REFERENCE_PROPERTY_KEY)
       || property == this->property(IDENTICAL_PROPERTY_KEY) ) {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

Object* Instance::referenced_object() const
{
  const auto reference = property(REFERENCE_PROPERTY_KEY)->value<ReferenceProperty::value_type>();
  Object* object_reference = static_cast<Object*>(reference);
  if (object_reference != nullptr && object_reference->is_ancestor_of(*this)) {
    LWARNING << "Instance cannot descend from referenced object.";
    return nullptr;
  } else {
    return object_reference;
  }
}

void Instance::update_tags()
{
  if (m_reference) {
    QSignalBlocker blocker(&scene()->mail_box());
    const auto instance_style_tags = type_cast<StyleTag*>(tags.ordered_items());
    if (!instance_style_tags.empty()) {
      for (Tag* tag : type_cast<StyleTag*>(m_reference->tags.ordered_items())) {
        m_reference->tags.remove(*tag);
      }
    }
    for (Tag* tag : instance_style_tags) {
      ListOwningContext<Tag> context(tag->clone(), m_reference->tags);
      m_reference->tags.insert(context);
    }
  }
}

}  // namespace omm
