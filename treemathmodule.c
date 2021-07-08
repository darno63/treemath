#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Python.h>

/* find gini impurity */
double giniImpurity(int *array, int length) {
  double arraySum = 0;
  for (int i=0; i<length;i++) {
    arraySum += array[i];
  }

  double fracSum = 0;
  for (int i=0; i<length;i++) {
    fracSum += pow(array[i] / arraySum,2);
  }
  
  return 1 - fracSum;
}

/* get children gini */
double getChildGini(double *xarray, int xlength, double *yarray, int ylength, double split, int nclasses) {

  /* create set of zeros */
  int classSetLeft[nclasses];
  int classSetRight[nclasses];
  for (int i=0;i<nclasses;i++) {
    classSetLeft[i] = 0;
    classSetRight[i] = 0;
  }

  /* loop through x array to build class set */
  int setLeftSize = 0;
  int setRightSize = 0;
  for (int i=0;i<xlength;i++) {
    if (xarray[i] <= split) {
      classSetLeft[(int)yarray[i]]++;
      setLeftSize ++;
    } else {
      classSetRight[(int)yarray[i]]++;
      setRightSize ++;
    }
  }

  /* get gini impurity of children */
  double LeftChildGini = giniImpurity(classSetLeft, nclasses);
  double RightChildGini = giniImpurity(classSetRight, nclasses);
  
  return (LeftChildGini * setLeftSize + RightChildGini * setRightSize) / (setLeftSize + setRightSize);
}

/* find split */
double* findSplit(double *xarray, int xlength, double *yarray, int ylength, int nclasses) {
  double gini;
  double mingini = 1;
  double* splitgini = malloc(sizeof(double)*2);

  double min = xarray[0];
  double max = xarray[0];
  for (int i=1;i<xlength;i++) {
    if ( xarray[i] < min){
      min = xarray[i];
    }
    
    if ( xarray[i] > max){
      max = xarray[i];
    }
  }

  double minsplit = max;
  double step = (max - min) / 50;
  for (double s=min;s<=max;s+=step) {
    gini = getChildGini(xarray, xlength, yarray, ylength, s, nclasses);
    if (gini < mingini) {
      minsplit = s;
      mingini = gini;
    }
  }
  
  splitgini[0] = minsplit;
  splitgini[1] = mingini;

  return splitgini;
}

/* python api */
static PyObject *py_findSplit(PyObject *self, PyObject *args) {
  PyObject *xbufobj, *ybufobj;
  Py_buffer xview, yview;
  int nclasses;
  double *result;
  /* Get the passed Python object */
  if (!PyArg_ParseTuple(args, "OOi", &xbufobj, &ybufobj, &nclasses)) {
    return NULL;
  }

  /* Attempt to extract buffer info from it */
  if (PyObject_GetBuffer(xbufobj, &xview, PyBUF_ANY_CONTIGUOUS | PyBUF_FORMAT) == -1) {
    return NULL;
  }
  if (PyObject_GetBuffer(ybufobj, &yview, PyBUF_ANY_CONTIGUOUS | PyBUF_FORMAT) == -1) {
    return NULL;
  }

  if (xview.ndim != 1) {
    PyErr_SetString(PyExc_TypeError, "Expected a 1-dimensional array");
    PyBuffer_Release(&xview);
    return NULL;
  }
  if (yview.ndim != 1) {
    PyErr_SetString(PyExc_TypeError, "Expected a 1-dimensional array");
    PyBuffer_Release(&yview);
    return NULL;
  }

  /* Check the type of items in the array */
  if (strcmp(xview.format, "d") != 0) {
    PyErr_SetString(PyExc_TypeError, "Expected an array of doubles");
    PyBuffer_Release(&xview);
    return NULL;
  }
  if (strcmp(yview.format, "d") != 0) {
    PyErr_SetString(PyExc_TypeError, "Expected an array of doubles");
    PyBuffer_Release(&yview);
    return NULL;
  }

  /* Pass the raw buffer and size to the C function */
  result = findSplit(xview.buf, xview.shape[0], yview.buf, yview.shape[0], nclasses);

  /* Indicate we're done working with the buffer */
  PyBuffer_Release(&xview);
  PyBuffer_Release(&yview);
  return Py_BuildValue("(dd)", result[0], result[1]);
}


/* Module method table */
static PyMethodDef TreeMethods[] = {
  {"find_split", py_findSplit, METH_VARARGS, "finds optimal threshold for split"},
  {NULL, NULL, 0, NULL}
};

/* Module structure */
static struct PyModuleDef treemathmodule = {
  PyModuleDef_HEAD_INIT,
  "treemath",
  "c calculations for decision trees",
  -1,
  TreeMethods
};

/* Module initialization function */
PyMODINIT_FUNC PyInit_treemath(void) {
  return PyModule_Create(&treemathmodule);
}
