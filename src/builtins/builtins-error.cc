// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/builtins/builtins.h"
#include "src/builtins/builtins-utils.h"

#include "src/messages.h"
#include "src/property-descriptor.h"
#include "src/string-builder.h"

namespace v8 {
namespace internal {

// ES6 section 19.5.1.1 Error ( message )
BUILTIN(ErrorConstructor) {
  HandleScope scope(isolate);
  RETURN_RESULT_OR_FAILURE(
      isolate,
      ErrorUtils::Construct(isolate, args.target<JSFunction>(),
                            Handle<Object>::cast(args.new_target()),
                            args.atOrUndefined(isolate, 1), SKIP_FIRST, false));
}

// static
BUILTIN(ErrorCaptureStackTrace) {
  HandleScope scope(isolate);
  Handle<Object> object_obj = args.atOrUndefined(isolate, 1);
  if (!object_obj->IsJSObject()) {
    THROW_NEW_ERROR_RETURN_FAILURE(
        isolate, NewTypeError(MessageTemplate::kInvalidArgument, object_obj));
  }
  Handle<JSObject> object = Handle<JSObject>::cast(object_obj);
  Handle<Object> caller = args.atOrUndefined(isolate, 2);
  FrameSkipMode mode = caller->IsJSFunction() ? SKIP_UNTIL_SEEN : SKIP_NONE;

  // Collect the stack trace.

  RETURN_FAILURE_ON_EXCEPTION(isolate,
                              isolate->CaptureAndSetDetailedStackTrace(object));

  // Eagerly format the stack trace and set the stack property.

  Handle<Object> stack_trace =
      isolate->CaptureSimpleStackTrace(object, mode, caller);
  if (!stack_trace->IsJSArray()) return *isolate->factory()->undefined_value();

  Handle<Object> formatted_stack_trace;
  ASSIGN_RETURN_FAILURE_ON_EXCEPTION(
      isolate, formatted_stack_trace,
      FormatStackTrace(isolate, object, stack_trace));

  PropertyDescriptor desc;
  desc.set_configurable(true);
  desc.set_writable(true);
  desc.set_value(formatted_stack_trace);
  Maybe<bool> status = JSReceiver::DefineOwnProperty(
      isolate, object, isolate->factory()->stack_string(), &desc,
      Object::THROW_ON_ERROR);
  if (!status.IsJust()) return isolate->heap()->exception();
  CHECK(status.FromJust());
  return isolate->heap()->undefined_value();
}

// ES6 section 19.5.3.4 Error.prototype.toString ( )
BUILTIN(ErrorPrototypeToString) {
  HandleScope scope(isolate);
  RETURN_RESULT_OR_FAILURE(isolate,
                           ErrorUtils::ToString(isolate, args.receiver()));
}

}  // namespace internal
}  // namespace v8
