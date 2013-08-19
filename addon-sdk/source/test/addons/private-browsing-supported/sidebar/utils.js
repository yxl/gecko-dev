/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
'use strict';

const { Cu } = require('chrome');
const { getMostRecentBrowserWindow } = require('sdk/window/utils');
const { fromIterator } = require('sdk/util/array');

const BLANK_IMG = exports.BLANK_IMG = 'data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==';

const BUILTIN_SIDEBAR_MENUITEMS = exports.BUILTIN_SIDEBAR_MENUITEMS = [
  'menu_socialSidebar',
  'menu_historySidebar',
  'menu_bookmarksSidebar'
];

function isSidebarShowing(window) {
  window = window || getMostRecentBrowserWindow();
  let sidebar = window.document.getElementById('sidebar-box');
  return !sidebar.hidden;
}
exports.isSidebarShowing = isSidebarShowing;

function getSidebarMenuitems(window) {
  window = window || getMostRecentBrowserWindow();
  return fromIterator(window.document.querySelectorAll('#viewSidebarMenu menuitem'));
}
exports.getSidebarMenuitems = getSidebarMenuitems;

function getExtraSidebarMenuitems() {
  let menuitems = getSidebarMenuitems();
  return menuitems.filter(function(mi) {
    return BUILTIN_SIDEBAR_MENUITEMS.indexOf(mi.getAttribute('id')) < 0;
  });
}
exports.getExtraSidebarMenuitems = getExtraSidebarMenuitems;

function makeID(id) {
  return 'jetpack-sidebar-' + id;
}
exports.makeID = makeID;

function simulateCommand(ele) {
  let window = ele.ownerDocument.defaultView;
  let { document } = window;
  var evt = document.createEvent('XULCommandEvent');
  evt.initCommandEvent('command', true, true, window,
    0, false, false, false, false, null);
  ele.dispatchEvent(evt);
}
exports.simulateCommand = simulateCommand;

function simulateClick(ele) {
  let window = ele.ownerDocument.defaultView;
  let { document } = window;
  let evt = document.createEvent('MouseEvents');
  evt.initMouseEvent('click', true, true, window,
    0, 0, 0, 0, 0, false, false, false, false, 0, null);
  ele.dispatchEvent(evt);
}
exports.simulateClick = simulateClick;

function getWidget(buttonId, window = getMostRecentBrowserWindow()) {
  const { CustomizableUI } = Cu.import('resource:///modules/CustomizableUI.jsm', {});
  const { AREA_NAVBAR } = CustomizableUI;

  let widgets = CustomizableUI.getWidgetsInArea(AREA_NAVBAR).
    filter(({id}) => id.startsWith('button--') && id.endsWith(buttonId));

  if (widgets.length === 0)
    throw new Error('Widget with id `' + buttonId +'` not found.');

  if (widgets.length > 1)
    throw new Error('Unexpected number of widgets: ' + widgets.length)

  return widgets[0].forWindow(window);
};
exports.getWidget = getWidget;

// OSX and Windows exhibit different behaviors when 'checked' is false,
// so compare against the consistent 'true'. See bug 894809.
function isChecked(node) {
  return node.getAttribute('checked') === 'true';
};
exports.isChecked = isChecked;
