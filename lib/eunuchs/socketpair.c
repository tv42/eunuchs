#include <Python.h>
#include <sys/types.h>
#include <sys/socket.h>

/* int socketpair(int domain, int type, int protocol, int sv[2]); */
static PyObject *my_socketpair(PyObject *self, PyObject *args, PyObject *keywds) {
  int family = AF_UNIX;
  int type = SOCK_STREAM;
  int proto = 0;
  int sv[2];
  int ret;

  static char *kwlist[] = {"family",
			   "type",
			   "proto",
			   NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "|iii", kwlist,
				   &family,
				   &type,
				   &proto))
    return NULL;
  ret = socketpair(family, type, proto, sv);
  if (ret<0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }

  return Py_BuildValue("ii", sv[0], sv[1]);
}

static PyMethodDef my_methods[] = {
  {
    "socketpair", (PyCFunction)my_socketpair, METH_VARARGS|METH_KEYWORDS,
    "socketpair([family[, type[, proto]]]) -> (fd, fd)"
  },

  {NULL, NULL, 0, NULL}
};

void initsocketpair(void) {
  Py_InitModule("socketpair", my_methods);
}
