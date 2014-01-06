/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const Cc = Components.classes;
const Ci = Components.interfaces;

function run_test()
{
  // Create the base directory.
  let base = Cc['@mozilla.org/file/directory_service;1']
             .getService(Ci.nsIProperties)
             .get('TmpD', Ci.nsILocalFile);
  base.append('renameTesting');
  if (base.exists()) {
    base.remove(true);
  }
  base.create(Ci.nsIFile.DIRECTORY_TYPE, parseInt('0777', 8));

  // Create a sub directory under the base.
  let subdir = base.clone();
  subdir.append('subdir');
  subdir.create(Ci.nsIFile.DIRECTORY_TYPE, parseInt('0777', 8));

  // Create a file under the sub directory.
  let tempFile = subdir.clone();
  tempFile.append('file0.txt');
  tempFile.createUnique(Ci.nsIFile.NORMAL_FILE_TYPE, parseInt('0777', 8));

  // Test rename in the base directory
  tempFile.rename(null, 'file1.txt');
  do_check_true(exists(subdir, 'file1.txt'));

  // Test moving across directories
  tempFile.rename(base, '');
  do_check_true(exists(base, 'file1.txt'));

  // Test moving across directories and renaming at the same time
  tempFile.rename(subdir, 'file2.txt');
  do_check_true(exists(subdir, 'file2.txt'));

  // Test moving a directory
  subdir.rename(base, 'renamed');
  do_check_true(exists(base, 'renamed'));
  do_check_true(exists(subdir, 'file2.txt'));

  base.remove(true);
}

function exists(parent, filename) {
  let file = parent.clone();
  file.append(filename);
  return file.exists();
}
