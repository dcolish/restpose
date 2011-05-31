/** @file tasks.cc
 * @brief Tasks to be placed on queues for performing later.
 */
/* Copyright (c) 2011 Richard Boulton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <config.h>
#include "tasks.h"

#include "httpserver/response.h"
#include "jsonxapian/collection.h"
#include "loadfile.h"
#include "server/task_manager.h"
#include "utils/jsonutils.h"
#include "utils/stringutils.h"

using namespace std;
using namespace RestPose;

Task::~Task() {}

void
StaticFileTask::perform(RestPose::Collection *)
{
    Response & response(resulthandle.response());
    string data;
    if (load_file(path, data)) {
	response.set_data(data);
	if (string_endswith(path, ".html")) {
	    response.set_content_type("text/html");
	} else if (string_endswith(path, ".js")) {
	    response.set_content_type("application/javascript");
	} else if (string_endswith(path, ".css")) {
	    response.set_content_type("text/css");
	} else if (string_endswith(path, ".png")) {
	    response.set_content_type("image/png");
	} else if (string_endswith(path, ".jpg")) {
	    response.set_content_type("image/jpeg");
	} else {
	    response.set_content_type("text/plain");
	}
	response.set_status(200);
    } else {
	Json::Value result(Json::objectValue);
	result["err"] = "Couldn't load file" + path; // FIXME - explain the error
	response.set(result, 404); // FIXME - only return 404 if it's a file not found error.
    }
    resulthandle.set_ready();
}

void
CollInfoTask::perform(RestPose::Collection * collection)
{
    Json::Value result(Json::objectValue);
    result["doc_count"] = Json::UInt64(collection->doc_count());
    Json::Value & types(result["types"] = Json::objectValue);
    // FIXME - set types to a map from typenames in the collection to their schemas.
    (void) types;

    resulthandle.response().set(result, 200);
    resulthandle.set_ready();
}

void
PerformSearchTask::perform(RestPose::Collection * collection)
{
    Json::Value result(Json::objectValue);
    collection->perform_search(search, result);
    resulthandle.response().set(result, 200);
    resulthandle.set_ready();
}

void
ServerStatusTask::perform(RestPose::Collection *)
{
    Json::Value result(Json::objectValue);
    Json::Value & tasks(result["tasks"] = Json::objectValue);
    {
	Json::Value & indexing(tasks["indexing"] = Json::objectValue);
	taskman->indexing_queues.get_status(indexing["queues"]);
	taskman->indexing_threads.get_status(indexing["threads"]);
    }
    {
	Json::Value & processing(tasks["processing"] = Json::objectValue);
	taskman->processing_queues.get_status(processing["queues"]);
	taskman->processing_threads.get_status(processing["threads"]);
    }
    {
	Json::Value & search(tasks["search"] = Json::objectValue);
	taskman->search_queues.get_status(search["queues"]);
	taskman->search_threads.get_status(search["threads"]);
    }
    resulthandle.response().set(result, 200);
    resulthandle.set_ready();
}

void
ProcessorPipeDocumentTask::perform(RestPose::Collection & collection,
				   TaskManager * taskman)
{
    collection.send_to_pipe(taskman, pipe, doc);
}

void
ProcessorProcessDocumentTask::perform(RestPose::Collection & collection,
				      TaskManager * taskman)
{
    string idterm;
    Xapian::Document xdoc = collection.process_doc(type, doc, idterm);
    taskman->queue_index_processed_doc(collection.get_name(), xdoc, idterm);
}

void
IndexerUpdateDocumentTask::perform(RestPose::Collection & collection)
{
    collection.raw_update_doc(doc, idterm);
}

IndexingTask *
IndexerUpdateDocumentTask::clone() const
{
    return new IndexerUpdateDocumentTask(idterm, doc);
}

void
IndexerDeleteDocumentTask::perform(RestPose::Collection & collection)
{
    collection.raw_delete_doc(idterm);
}

IndexingTask *
IndexerDeleteDocumentTask::clone() const
{
    return new IndexerDeleteDocumentTask(idterm);
}

void
IndexerCommitTask::perform(RestPose::Collection & collection)
{
    collection.commit();
}

IndexingTask *
IndexerCommitTask::clone() const
{
    return new IndexerCommitTask();
}
