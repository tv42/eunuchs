#include <poll.h>
#include "epoll.h"
#include <sys/errno.h>
#include <Python.h>

typedef struct {
  PyObject_HEAD
  struct pollfd const *events;
  int count;
} PyEpollResultObject;

static PyTypeObject PyEpollResult_Type = {
  PyObject_HEAD_INIT(0)         /* Must fill in type value later */
  .tp_name = "_epoll.FOO",
  .tp_basicsize = sizeof(PyEpollResult_Type),
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
};

/* int epoll_create(int size); */
static PyObject *my_epoll_create(PyObject *self, PyObject *args) {
  int size;
  int ret;

  if (!PyArg_ParseTuple(args, "i", &size))
    return NULL;
  ret = sys_epoll_create(size);
  if (ret<0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  return Py_BuildValue("i", ret);
}

/* int epoll_ctl(int epfd, int op, int fd, unsigned int events); */
static PyObject *my_epoll_ctl(PyObject *self, PyObject *args) {
  int epfd, op, fd;
  unsigned int events;
  int ret;

  if (!PyArg_ParseTuple(args, "iiii", &epfd, &op, &fd, &events))
    return NULL;
  ret = sys_epoll_ctl(epfd, op, fd, events);
  if (ret<0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *my_epoll_iter(PyObject *self) {
  PyEpollResultObject *state;
  PyObject *result;

  state = (PyEpollResultObject *)self;

  if (!state->count) {
    PyErr_SetObject(PyExc_StopIteration, Py_None);
    return NULL;
  }

  result = Py_BuildValue("(ii)", state->events->fd,
			 state->events->revents);

  if (result) {
    state->count--;
    state->events++;
  }
  return result;
}

/* int epoll_wait(int epfd, struct pollfd *events, int maxevents,
   int timeout); */
static PyObject *my_epoll_wait(PyObject *self, PyObject *args) {
  int epfd, timeout;
  struct pollfd const *events;
  int ret;

  if (!PyArg_ParseTuple(args, "iw#i", &epfd, &events, &timeout))
    return NULL;
  ret = sys_epoll_wait(epfd, &events, timeout);
  if (ret<0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }

  /* create an iterator out of events and ret */
  {
    PyEpollResultObject *o;

    o = (PyEpollResultObject *)
      PyType_GenericNew(&PyEpollResult_Type, NULL, NULL);
    if (o) {
      o->events = events;
      o->count = ret;
    }
    return (PyObject *)o;
  }
}


static PyMethodDef my_methods[] = {
  {
    "epoll_create", my_epoll_create, METH_VARARGS,
    "epoll_create(size) -> fd"
  },
  {
    "epoll_ctl", my_epoll_ctl, METH_VARARGS,
    "epoll_ctl(epfd, op, fd, events)"
  },
  {
    "epoll_wait", my_epoll_wait, METH_VARARGS,
    "epoll_wait(epfd, buffer, timeout) -> fd"
  },

  {NULL, NULL, 0, NULL}
};

/* Utility function to add integer constant to module dictionary */
static void insertConstant(PyObject *d, char *name, unsigned long code)
{
  PyObject *u = PyString_FromString(name);
  PyObject *v = PyInt_FromLong((long) code);

  /* Don't bother checking for errors; they'll be caught at the end
   * of the module initialization function by the caller of
   * initwin32inet().
   */
  if (u && v) {
    /* insert in modules dict */
    PyDict_SetItem(d, u, v);
  }
  Py_XDECREF(u);
  Py_XDECREF(v);
}

void initepoll(void) {
  PyObject *module, *dict;

  PyEpollResult_Type.ob_type = &PyType_Type;
  PyEpollResult_Type.tp_getattro = PyObject_GenericGetAttr;
  PyEpollResult_Type.tp_alloc = PyType_GenericAlloc;
  PyEpollResult_Type.tp_free = _PyObject_Del;

  module = Py_InitModule("epoll", my_methods);
  dict = PyModule_GetDict(module);
  if (!dict) {
    return;
  }

  insertConstant(dict, "EP_CTL_ADD", EP_CTL_ADD);
  insertConstant(dict, "EP_CTL_MOD", EP_CTL_MOD);
  insertConstant(dict, "EP_CTL_DEL", EP_CTL_DEL);
}
