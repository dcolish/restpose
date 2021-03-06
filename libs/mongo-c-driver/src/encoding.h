/*
 * Copyright 2009-2010 10gen, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _BSON_ENCODING_H_
#define _BSON_ENCODING_H_

MONGO_EXTERN_C_START

#define BSON_VALID 0x0
#define BSON_NOT_UTF8 0x2
#define BSON_FIELD_HAS_DOT 0x4
#define BSON_FIELD_INIT_DOLLAR 0x8

/**
 * Check that a field name is valid UTF8, does not start with a '$',
 * and contains no '.' characters. Set buffer bit field appropriately.
 * Note that we don't need to check for '\0' because we're using
 * strlen(3), which stops at '\0'.
 *
 * @param b The bson_buffer to which field name will be appended.
 * @param string The field name as char*.
 * @param length The length of the field name.
 *
 * @return 1 if valid UTF8 and 0 if not. All BSON strings must be
 *     valid UTF8. Note that this function may return 1 if string
 *     contains '.', since the validity of this depends on context.
 */
bson_bool_t bson_check_field_name( bson_buffer* b, const unsigned char* string,
    const int length );

/**
 * Check that a string is valid UTF8. Sets the buffer bit field appropriately.
 *
 * @param b The bson_buffer to which string will be appended.
 * @param string The string to check.
 * @param length The length of the string.
 *
 * @return 1 if valid UTF8 and 0 if not.
 */
bson_bool_t bson_check_string( bson_buffer* b, const unsigned char* string,
    const int length );

MONGO_EXTERN_C_END
#endif
