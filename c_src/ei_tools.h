#ifndef EI_TOOLS
#define EI_TOOLS

#define ATOM_TRUE   "true"
#define ATOM_FALSE  "false"
#define ATOM_OK     "ok"
#define ATOM_ERROR  "error"
#define ATOM_BADARG "badarg"
#define ATOM_NOMEM  "nomem"
#define ATOM_BUSY   "busy"

#define EI_DECODE_VERSION(uPort, uBuffer, uIndex, uVersion)     \
  if (ei_decode_version (uBuffer, &uIndex, &uVersion) == -1)    \
    {                                                           \
      driver_failure_atom (uPort, "ei_decode_version");         \
      return;                                                   \
    }

#define EI_DECODE_LONG(uPort, uBuffer, uIndex, uLong)   \
  if (ei_decode_long (uBuffer, &uIndex, &uLong) == -1)  \
    {                                                   \
      driver_failure_atom (uPort, "ei_decode_long");    \
      return;                                           \
    }

#define EI_DECODE_ULONG(uPort, uBuffer, uIndex, uLong)  \
  if (ei_decode_ulong (uBuffer, &uIndex, &uLong) == -1) \
    {                                                   \
      driver_failure_atom (uPort, "ei_decode_ulong");   \
      return;                                           \
    }

#define EI_DECODE_DOUBLE(uPort, uBuffer, uIndex, uDouble)       \
  if (ei_decode_double (uBuffer, &uIndex, &uDouble) == -1)      \
    {                                                           \
      driver_failure_atom (uPort, "ei_decode_double");          \
      return;                                                   \
    }

#define EI_DECODE_BOOLEAN(uPort, uBuffer, uIndex, uBoolean)     \
  if (ei_decode_boolean (uBuffer, &uIndex, &uBoolean) == -1)    \
    {                                                           \
      driver_failure_atom (uPort, "ei_decode_boolean");         \
      return;                                                   \
    }

#define EI_DECODE_CHAR(uPort, uBuffer, uIndex, uChar)   \
  if (ei_decode_char (uBuffer, &uIndex, &uChar) == -1)  \
    {                                                   \
      driver_failure_atom (uPort, "ei_decode_char");    \
      return;                                           \
    }

#define EI_DECODE_STRING(uPort, uBuffer, uIndex, uString)       \
  if (ei_decode_string (uBuffer, &uIndex, uString) == -1)       \
    {                                                           \
      driver_failure_atom (uPort, "ei_decode_string");          \
      return;                                                   \
    }

#define EI_DECODE_ATOM(uPort, uBuffer, uIndex, uAtom)   \
  if (ei_decode_atom (uBuffer, &uIndex, uAtom) == -1)   \
    {                                                   \
      driver_failure_atom (uPort, "ei_decode_atom");    \
      return;                                           \
    }

#define EI_DECODE_BINARY(uPort, uBuffer, uIndex, uBinary, uLength)      \
  if (ei_decode_binary (uBuffer, &uIndex, uBinary, &uLength) == -1)     \
    {                                                                   \
      driver_failure_atom (uPort, "ei_decode_binary");                  \
      return;                                                           \
    }

#define EI_DECODE_TUPLE_HEADER(uPort, uBuffer, uIndex, uArity)  \
  if (ei_decode_tuple_header (uBuffer, &uIndex, &uArity) == -1) \
    {                                                           \
      driver_failure_atom (uPort, "ei_decode_tuple_header");    \
      return;                                                   \
    }

#define EI_DECODE_LIST_HEADER(uPort, uBuffer, uIndex, uArity)   \
  if (ei_decode_list_header (uBuffer, &uIndex, &uArity) == -1)  \
    {                                                           \
      driver_failure_atom (uPort, "ei_decode_list_header");     \
      return;                                                   \
    }

#define EI_DECODE_INTLIST(uPort, uBuffer, uIndex, uArray, uCount)       \
  if (ei_decode_intlist (uBuffer, &uIndex, uArray, &uCount) == -1)      \
    {                                                                   \
      driver_failure_atom (uPort, "ei_decode_intlist");                 \
      return;                                                           \
    }

#define ASSERT_DRIVER_ALLOC(uPort, uPointer, uLabel)    \
  if (uPointer == NULL)                                 \
    {                                                   \
      driver_failure_atom (uPort, "driver_alloc");      \
      goto uLabel;                                      \
    }

#define ASSERT_DRIVER_ALLOC_BINARY(uPort, uPointer, uLabel)     \
  if (uPointer == NULL)                                         \
    {                                                           \
      driver_failure_atom (uPort, "driver_alloc_binary");       \
      goto uLabel;                                              \
    }

#define EI_GET_TYPE(uPort, uBuffer, uIndex, uValueType, uValueSize)     \
  int uValueType, uValueSize;                                           \
  if (ei_get_type (uBuffer, &uIndex, &uValueType, &uValueSize) == -1)   \
    {                                                                   \
      driver_failure_atom (uPort, "ei_get_type");                       \
      return;                                                           \
    }

#endif
