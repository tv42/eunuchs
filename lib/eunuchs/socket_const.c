#include <Python.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

static PyMethodDef my_methods[] = {
  {NULL, NULL, 0, NULL}
};

/* Utility function to add integer constant to module dictionary */
static void insertConstant(PyObject *d, char *name, unsigned long code)
{
  PyObject *u = PyString_FromString(name);
  PyObject *v = PyInt_FromLong((long) code);

  if (u && v) {
    /* insert in modules dict */
    PyDict_SetItem(d, u, v);
  }
  Py_XDECREF(u);
  Py_XDECREF(v);
}

void initsocket_const(void) {
  PyObject *module, *dict;

  module = Py_InitModule("socket_const", my_methods);
  dict = PyModule_GetDict(module);
  if (!dict) {
    return;
  }

  insertConstant(dict, "SCM_RIGHTS", SCM_RIGHTS);
  insertConstant(dict, "SCM_CREDENTIALS", SCM_CREDENTIALS);
}
