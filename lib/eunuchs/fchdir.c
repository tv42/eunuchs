#include <Python.h>
#include <unistd.h>

/* int fchdir(int fd); */
static PyObject *my_fchdir(PyObject *self, PyObject *args) {
  int fd;
  int ret;

  if (!PyArg_ParseTuple(args, "i", &fd))
    return NULL;
  ret = fchdir(fd);
  if (ret<0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef my_methods[] = {
  {
    "fchdir", my_fchdir, METH_VARARGS,
    "fchdir(fd)"
  },

  {NULL, NULL, 0, NULL}
};

void initfchdir(void) {
  Py_InitModule("fchdir", my_methods);
}
