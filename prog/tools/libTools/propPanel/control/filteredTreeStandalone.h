// Copyright (C) Gaijin Games KFT.  All rights reserved.
#pragma once

#include <libTools/util/strUtil.h>
#include <propPanel/control/filteredTreeTypes.h>
#include "treeStandalone.h"

namespace PropPanel
{

class FilteredTreeControlStandalone : public ITreeInterface
{
public:
  // The original Windows-based control sent events even when the selection was changed by code, not just when it was
  // set by the user through the UI. This behaviour can be controlled with selection_change_events_by_code.
  explicit FilteredTreeControlStandalone(bool selection_change_events_by_code, bool has_checkboxes = false,
    bool multi_select = false) :
    tree(selection_change_events_by_code, has_checkboxes, multi_select), rootNode(nullptr, BAD_TEXTUREID, nullptr, nullptr)
  {}

  void setEventHandler(WindowControlEventHandler *event_handler) { tree.setEventHandler(event_handler); }

  // The provided window_base is passed to the event handler events (set with setEventHandler). This class does not use
  // it otherwise.
  void setWindowBaseForEventHandler(WindowBase *window_base) { tree.setWindowBaseForEventHandler(window_base); }

  bool isEnabled() const { return tree.isEnabled(); }

  void setEnabled(bool enabled) { tree.setEnabled(enabled); }

  void setFocus() { tree.setFocus(); }

  void setCheckboxIcons(TEXTUREID checked, TEXTUREID unchecked) { tree.setCheckboxIcons(checked, unchecked); }

  TLeafHandle addItem(const char *name, TEXTUREID icon = BAD_TEXTUREID, TLeafHandle parent = nullptr, void *user_data = nullptr)
  {
    return addItemInternal(name, icon, parent, user_data, false);
  }

  TLeafHandle addItem(const char *name, const char *icon_name, TLeafHandle parent = nullptr, void *user_data = nullptr)
  {
    TEXTUREID icon = tree.getTexture(icon_name);
    return addItemInternal(name, icon, parent, user_data, false);
  }

  TLeafHandle addItemAsFirst(const char *name, TEXTUREID icon = BAD_TEXTUREID, TLeafHandle parent = nullptr, void *user_data = nullptr)
  {
    return addItemInternal(name, icon, parent, user_data, true);
  }

  TLeafHandle addItemAsFirst(const char *name, const char *icon_name, TLeafHandle parent = nullptr, void *user_data = nullptr)
  {
    TEXTUREID icon = tree.getTexture(icon_name);
    return addItemInternal(name, icon, parent, user_data, true);
  }

  void clear() { tree.clear(); }

  bool removeItem(TLeafHandle item)
  {
    TTreeNode *node = getItemNode(item);
    if (!node)
      return false;

    tree.removeLeaf(item);

    TTreeNode *parent = node->parent ? node->parent : &rootNode;
    auto it = eastl::find(parent->nodes.begin(), parent->nodes.end(), node);
    G_ASSERT(it != parent->nodes.end());
    parent->nodes.erase(it);
    delete node;

    return true;
  }

  bool swapLeaf(TLeafHandle id, bool after) { return tree.swapLeaf(id, after); }

  void setTitle(TLeafHandle leaf, const char value[]) { tree.setTitle(leaf, value); }

  void setButtonPictures(TLeafHandle leaf, const char *fname = NULL) { tree.setButtonPictures(leaf, fname); }

  void setColor(TLeafHandle leaf, E3DCOLOR value) { tree.setColor(leaf, value); }

  bool isExpanded(TLeafHandle leaf) const { return tree.isExpanded(leaf); }

  void setExpanded(TLeafHandle leaf, bool open = true) { tree.setExpanded(leaf, open); }

  void setExpandedRecursive(TLeafHandle leaf, bool open = true) { tree.setExpandedRecursive(leaf, open); }

  void setExpandedTillRoot(TLeafHandle leaf, bool open = true) { tree.setExpandedTillRoot(leaf, open); }

  void setUserData(TLeafHandle leaf, void *value)
  {
    TTreeNode *node = getItemNode(leaf);
    if (!node)
      return;

    node->userData = value;
  }

  void setCheckboxState(TLeafHandle leaf, TreeControlStandalone::CheckboxState state) { tree.setCheckboxState(leaf, state); }

  void setIcon(TLeafHandle leaf, TEXTUREID icon) { tree.setIcon(leaf, icon); }

  void setStateIcon(TLeafHandle leaf, TEXTUREID icon) { tree.setStateIcon(leaf, icon); }

  int getChildCount(TLeafHandle leaf) const { return tree.getChildCount(leaf); }

  TEXTUREID getTexture(const char *name) { return tree.getTexture(name); }

  TLeafHandle getChildLeaf(TLeafHandle leaf, unsigned idx) const { return tree.getChildLeaf(leaf, idx); }

  TLeafHandle getParentLeaf(TLeafHandle leaf) const { return tree.getParentLeaf(leaf); }

  TLeafHandle getNextLeaf(TLeafHandle leaf) const { return tree.getNextLeaf(leaf); }

  TLeafHandle getPrevLeaf(TLeafHandle leaf) const { return tree.getPrevLeaf(leaf); }

  TLeafHandle getNextNode(TLeafHandle item, bool forward) const { return forward ? getNextLeaf(item) : getPrevLeaf(item); }

  TLeafHandle getRootLeaf() const { return tree.getRootLeaf(); }

  const String *getItemTitle(TLeafHandle leaf) const { return tree.getItemTitle(leaf); }

  TTreeNode *getItemNode(TLeafHandle p)
  {
    void *userData = tree.getUserData(p);
    return static_cast<TTreeNode *>(userData);
  }

  const TTreeNode *getItemNode(TLeafHandle p) const
  {
    void *userData = tree.getUserData(p);
    return static_cast<const TTreeNode *>(userData);
  }

  void *getItemData(TLeafHandle item) const
  {
    const TTreeNode *node = getItemNode(item);
    return node ? node->userData : nullptr;
  }

  TreeControlStandalone::CheckboxState getCheckboxState(TLeafHandle leaf) const { return tree.getCheckboxState(leaf); }

  bool isLeafSelected(TLeafHandle leaf) const { return tree.isLeafSelected(leaf); }

  void setSelectedLeaf(TLeafHandle leaf) { tree.setSelectedLeaf(leaf); }

  TLeafHandle getSelectedLeaf() const { return tree.getSelectedLeaf(); }

  bool isItemMultiSelected(TLeafHandle leaf) const { return tree.isItemMultiSelected(leaf); }

  void getSelectedLeafs(dag::Vector<TLeafHandle> &leafs) const { tree.getSelectedLeafs(leafs); }

  void ensureVisible(TLeafHandle leaf) { tree.ensureVisible(leaf); }

  void addChildName(const char *name, TLeafHandle parent)
  {
    TTreeNode *node = getItemNode(parent);
    if (!node)
      return;

    node->childNames.emplace_back(name);
  }

  Tab<String> getChildNames(TLeafHandle item)
  {
    TTreeNode *treeNode = getItemNode(item);
    if (treeNode)
      return treeNode->childNames;

    return Tab<String>();
  }

  void filter(void *param, TTreeNodeFilterFunc filter_func)
  {
    TTreeNode *originalSelection = getItemNode(getSelectedLeaf());

    // Get expansion state. It is needed for filterRoutine.
    TLeafHandle root = getRootLeaf();
    TLeafHandle leaf = root;
    while (true)
    {
      TTreeNode *node = getItemNode(leaf);
      if (node)
        node->isExpand = isExpanded(leaf);

      leaf = getNextNode(leaf, true);
      if (!leaf || leaf == root)
        break;
    }

    tree.clear();
    filterRoutine(&rootNode, 0, param, filter_func, originalSelection);

    TLeafHandle selection = tree.getSelectedLeaf();
    if (selection)
      tree.ensureVisible(selection);
  }

  TLeafHandle search(const char *text, TLeafHandle first, bool forward, bool use_wildcard_search = false)
  {
    const String searchText = String(text).toLower();
    TLeafHandle next = first;

    while (true)
    {
      next = getNextNode(next, forward);
      if (!next || next == first)
        return 0;

      TTreeNode *node = getItemNode(next);
      G_ASSERT(node);
      if (!node)
        continue;

      if (searchText.empty())
        return next;

      if (use_wildcard_search)
      {
        if (str_matches_wildcard_pattern(String(node->name).toLower().c_str(), searchText.c_str()))
          return next;
      }
      else
      {
        if (strstr(String(node->name).toLower().c_str(), searchText.c_str()))
          return next;
      }
    }
  }

  virtual IMenu &createContextMenu() override { return tree.createContextMenu(); }

  void setMessage(const char *in_message) { tree.setMessage(in_message); }

  // If control_height is 0 then it will use the entire available height.
  void updateImgui(float control_height = 0.0f) { tree.updateImgui(control_height); }

private:
  TLeafHandle addItemInternal(const char *name, TEXTUREID icon, TLeafHandle parent, void *user_data, bool as_first)
  {
    TTreeNode *parentNode = nullptr;
    if (parent)
    {
      parentNode = getItemNode(parent);
      G_ASSERT(parentNode);
    }

    TTreeNode *newNode = new TTreeNode(name, icon, user_data, parentNode);
    if (parent)
      parentNode->nodes.push_back(newNode); //-V522 (Possible null pointer dereference.)
    else
      rootNode.nodes.push_back(newNode);

    TLeafHandle item;
    if (as_first)
      newNode->item = tree.createTreeLeafAsFirst(parent, name, icon, newNode);
    else
      newNode->item = tree.createTreeLeaf(parent, name, icon, newNode);

    return newNode->item;
  }

  bool filterRoutine(TTreeNode *node, TLeafHandle parent, void *param, TTreeNodeFilterFunc filter_func, TTreeNode *select_node)
  {
    G_ASSERT(filter_func);

    bool hasItems = false;

    for (int i = 0; i < node->nodes.size(); ++i)
    {
      TTreeNode &nd = *node->nodes[i];

      bool isVisible = filter_func(param, nd);

      nd.item = nullptr;
      if (!isVisible && !nd.nodes.size())
        continue;

      TLeafHandle item = tree.createTreeLeaf(parent, nd.name.c_str(), nd.icon, &nd);
      nd.item = item;

      if (nd.nodes.size())
      {
        bool res = filterRoutine(&nd, item, param, filter_func, select_node);

        if (!res && !isVisible)
        {
          nd.item = nullptr;
          tree.removeLeaf(item);
          continue;
        }
      }

      if (nd.isExpand)
        tree.setExpanded(item, nd.isExpand);

      if (&nd == select_node)
        tree.setSelectedLeaf(item);

      hasItems = true;
    }

    return hasItems;
  }

  TreeControlStandalone tree;
  TTreeNode rootNode;
};

} // namespace PropPanel