/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface File;

/*
 * All functions on Directory that accept DOMString arguments for file or
 * directory names only allow relative path to current directory itself. The
 * path should be a descendent path like "path/to/file.txt" and not contain a
 * segment of ".." or ".". So the paths aren't allowd to walk up the directory
 * tree. For example, paths like "../foo", "..", "/foo/bar" or "foo/../bar" are
 * not allowed.
 */
interface Directory {
  /*
   * The leaf name of the directory.
   */
  readonly attribute DOMString name;

  /*
   * Creates a new file or replaces an existing file with given data. The file
   * should be a descendent of current directory.
   *
   * @param path The relative path of the new file to current directory.
   * @param options It has two optional properties, 'ifExists' and 'data'.
   * If 'ifExists' is 'fail' and the path already exists, createFile must fail;
   * If 'ifExists' is 'replace', the path already exists, and is a file, create
   * a new file to replace the existing one;
   * If 'ifExists' is 'replace', the path already exists, but is a directory,
   * createFile must fail.
   * Otherwise, if no other error occurs, createFile will create a new file.
   * The 'data' property contains the new file's content.
   * @return If succeeds, the promise is resolved with the new created
   * File object. Otherwise, rejected with a DOM error.
   */
  [NewObject]
  // Promise<File>
  Promise createFile(DOMString path, optional CreateFileOptions options);

  /*
   * Creates a descendent directory. This method will create any intermediate
   * directories specified by the path segments.
   *
   * @param path The relative path of the new directory to current directory.
   * If path exists, createDirectory must fail.
   * @return If succeeds, the promise is resolved with the new created
   * Directory object. Otherwise, rejected with a DOM error.
   */
  [NewObject]
  // Promise<Directory>
  Promise createDirectory(DOMString path);

  /*
   * Gets a descendent file or directory with the given path.
   *
   * @param path The descendent entry's relative path to current directory.
   * @return If the path exists and no error occurs, the promise is resolved
   * with a File or Directory object, depending on the entry's type. Otherwise,
   * rejected with a DOM error.
   */
  [NewObject]
  // Promise<(File or Directory)>
  Promise get(DOMString path);

  /*
   * Moves (renames) a file or directory. The source path must be contained in
   * current directory or its descendents.
   *
   * @param path The source. It can be a relative path string (to current
   * directory), a File or Directory object.
   * @param dest The destination. If a DOM string is passed, it is the relative
   * path to current directory and can't contain '..' segment. If a Directory
   * object is passed, the source will be moved to that directory. If a
   * DestinationDict is passed, the parent directory and the new name of the
   * destination must be specified.
   * @return If the source doesn't exist or the destination already exists, move
   * should fail and the returning AbortableProgressPromise is rejected with a
   * DOM error. During the operation, the current file being moved will be sent
   * as progress value of the AbortableProgressPromise.
   *
   * It should be
   *   AbortableProgressPromise move((DOMString or File or Directory) path,
   *     optional (DOMString or Directory or DestinationDict) dest);
   * but gecko currently requires the last union argument with Dictionary
   * memeber to be optional.
   */
  [NewObject]
  AbortableProgressPromise
    move((DOMString or File or Directory) path,
         optional (DOMString or Directory or DestinationDict) dest);

  /*
   * Deletes a file or an empty directory. The target must be a descendent of
   * current directory.
   * @param path If a DOM string is passed, it is the relative path of the
   * target. Otherwise, the File or Directory object of the target should be
   * passed.
   * @return If the target is a non-empty directory, or if deleting the target
   * fails, the promise is rejected with a DOM error. If the target did not
   * exist, the promise is resolved with boolean false. If the target did exist
   * and was successfully deleted, the promise is resolved with boolean true.
   */
  [NewObject]
  // Promise<boolean>
  Promise remove((DOMString or File or Directory) path);

  /*
   * Deletes a file or a directory recursively. The target should be a
   * descendent of current directory.
   * @param path If a DOM string is passed, it is the relative path of the
   * target. Otherwise, the File or Directory object of the target should be
   * passed.
   * @return If the target exists, but deleting the target fails, the promise is
   * rejected with a DOM error. If the target did not exist, the promise is
   * resolved with boolean false. If the target did exist and was successfully
   * deleted, the promise is resolved with boolean true.
   */
  [NewObject]
  // Promise<boolean>
  Promise removeDeep((DOMString or File or Directory) path);

  /*
   * Open a file for reading. The file should be a descendent of current
   * directory.
   * @param path If a DOM string is passed, it is the relative path of the file.
   * Otherwise, a File object should be passed.
   * @return If the target is a File and no other error occurs, the promise is
   * resolved with a FileHandle object. Otherwise, rejected with a DOM error.
   */
  [NewObject]
  // Promise<FileHandle>
  Promise openRead((DOMString or File) path);

  /*
   * Open a file for modifying. The file should be a descendent of current
   * directory.
   * @param path If a DOM string is passed, it is the relative path of the file.
   * Otherwise, a File object should be passed.
   * @param options It has two optional properties, 'ifNotExists' and
   * 'ifExists'.
   * If the target file doesn't exist, and 'ifNotExists' is 'create', the target
   * file should be created and opened.
   * If the target file exists, and 'ifExists' is 'open', the target file should
   * be opened.
   * For other cases, openWrite should fail.
   * @return If the target is a File and no other error occurs, the promise is
   * is resolved with a FileHandleWritable object. Otherwise, rejected with a
   * DOM error.
   */
  [NewObject]
  // Promise<FileHandleWritable>
  Promise openWrite((DOMString or File) path, optional OpenWriteOptions options);

  /*
   * Gets all files or sub-directories under the target directory.
   * @param path If empty string or no value is passed, set current directory as
   * the target directory. Otherwise the relative path of the target directory,
   * which must be a descendent of current directory, should be passed.
   * @return If the target is not a directory or other error occurs, the promise
   * is rejected with a DOM error. Otherwise, resolved with an EventStream
   * object for getting the File and Directory objects.
   */
  // EventStream<(File or Directory)>
  EventStream enumerate(optional DOMString path);

  /*
   * Get all files under the target directory and its descendent directories.
   * @param path If empty string or no value is passed, set current directory as
   * the target directory. Otherwise the relative path of the target directory,
   * which must be a descendent of current directory, should be passed.
   * @return If the target is not a directory or other error occurs, the promise
   * is rejected with a DOM error. Otherwise, resolved with an EventStream
   * object for getting the File objects.
   */
  // EventStream<File> enumerateDeep
  EventStream enumerateDeep(optional DOMString path);
};

callback ForEachCallback = Promise? (optional any value);
interface EventStream {
  Promise forEach(ForEachCallback callback);
};

dictionary OpenWriteOptions {
  OpenIfNotExistsMode ifNotExists = "create";
  OpenIfExistsMode ifExists = "open";
};

enum CreateIfExistsMode { "replace", "fail" };
enum OpenIfExistsMode { "open", "fail" };
enum OpenIfNotExistsMode { "create", "fail" };

dictionary DestinationDict {
  // The parent directory.
  Directory dir;
  // The file name.
  DOMString name;
};

enum FileOpenMode { "readonly", "readwrite" };

