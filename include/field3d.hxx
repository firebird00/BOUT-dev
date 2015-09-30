/**************************************************************************
 * Copyright 2010 B.D.Dudson, S.Farley, M.V.Umansky, X.Q.Xu
 *
 * Contact: Ben Dudson, bd512@york.ac.uk
 * 
 * This file is part of BOUT++.
 *
 * BOUT++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BOUT++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOUT++.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************/

class Field3D;

#pragma once
#ifndef __FIELD3D_H__
#define __FIELD3D_H__

#include "field.hxx"
#include "field2d.hxx"
#include "fieldperp.hxx"
#include "stencils.hxx"
#include "bout_types.hxx"

#include "bout/dataiterator.hxx"

#include "bout/deprecated.hxx"

#include "bout/field_visitor.hxx"

/// Structure to store blocks of memory for Field3D class
struct memblock3d {
  /// memory block
  BoutReal ***data;

  /// Number of references
  int refs;
  
  /// Pointer to next block in linked-list structure
  memblock3d *next;
  
  /// Pointer in list of all blocks
  memblock3d *all_next;
}; 


/// Class for 3D X-Y-Z scalar fields
/*!
  Recycles memory using a global stack. Overloaded operators
  provide operations on data.

  July 2008: Added FieldData virtual functions
  May 2008: Added reference counting to reduce memory copying
 */
class Field3D : public Field, public FieldData {
 public:
  /// Constructor
  Field3D();
  /// copy constructor
  Field3D(const Field3D& f);
  /// Constructor from 2D field
  Field3D(const Field2D& f);
  /// Constructor from value
  Field3D(const BoutReal val);
  /// Destructor
  ~Field3D();

  /// Data type
  using value_type = BoutReal;
  
  DEPRECATED(Field3D* clone() const);

  /// Ensures that memory is allocated
  void allocate() const;
  
  /// Test if data is allocated
  bool isAllocated() const { return block !=  NULL; } 
  
  /// Return a pointer to the time-derivative field
  Field3D* timeDeriv();

  /*!
   * Ensure that this field has separate fields
   * for yup and ydown.
   */
  void splitYupYdown();

  /*!
   * Ensure that yup and ydown refer to this field
   */
  void mergeYupYdown();
  
  /// Flux Coordinate Independent (FCI) method
  Field3D& yup() { return *yup_field; }
  const Field3D& yup() const { return *yup_field; }
  
  Field3D& ydown() { return *ydown_field; }
  const Field3D& ydown() const { return *ydown_field; }

  // Staggered grids
  void setLocation(CELL_LOC loc); // Set variable location
  CELL_LOC getLocation() const; // Variable location
  
  /////////////////////////////////////////////////////////
  // Data access
  
  const DataIterator iterator() const;

  const DataIterator begin() const;
  const DataIterator end() const;
  
  BoutReal& operator[](DataIterator &d) {
    return operator()(d.x, d.y, d.z);
  }
  const BoutReal& operator[](DataIterator &d) const {
    return operator()(d.x, d.y, d.z);
  }
  BoutReal& operator[](const Indices &i) {
    return operator()(i.x, i.y, i.z);
  }
  const BoutReal& operator[](const Indices &i) const {
    return operator()(i.x, i.y, i.z);
  }
  
  /// Allows access to internal data using square-brackets
  DEPRECATED(BoutReal** operator[](int jx) const);
  
  BoutReal& operator[](bindex &bx);
  const BoutReal& operator[](bindex &bx) const;
  
  inline BoutReal& operator()(int jx, int jy, int jz) {
#if CHECK > 2
    // Perform bounds checking
    if(block == NULL)
      throw BoutException("Field3D: () operator on empty data");
    
    if((jx < 0) || (jx >= mesh->ngx) || 
       (jy < 0) || (jy >= mesh->ngy) || 
       (jz < 0) || (jz >= mesh->ngz))
      throw BoutException("Field3D: (%d, %d, %d) operator out of bounds (%d, %d, %d)", 
			  jx, jy, jz, mesh->ngx, mesh->ngy, mesh->ngz);
#endif
    return block->data[jx][jy][jz];
  }
  
  inline const BoutReal& operator()(int jx, int jy, int jz) const {
#if CHECK > 2
    if(block == NULL)
      throw BoutException("Field3D: () operator on empty data");
    
    if((jx < 0) || (jx >= mesh->ngx) || 
       (jy < 0) || (jy >= mesh->ngy) || 
       (jz < 0) || (jz >= mesh->ngz))
      throw BoutException("Field3D: (%d, %d, %d) operator out of bounds (%d, %d, %d)", 
			  jx, jy, jz, mesh->ngx, mesh->ngy, mesh->ngz);
#endif
    return block->data[jx][jy][jz];
  }

  inline BoutReal* operator()(int jx, int jy) const {
#if CHECK > 2
    if(block == NULL)
      throw BoutException("Field3D: () operator on empty data");

    if((jx < 0) || (jx >= mesh->ngx) ||
       (jy < 0) || (jy >= mesh->ngy))
      throw BoutException("Field3D: (%d, %d) operator out of bounds (%d, %d)",
                          jx, jy, mesh->ngx, mesh->ngy);
#endif
    return block->data[jx][jy];
  }
  
  /////////////////////////////////////////////////////////
  // Operators

  /// Assignment operators
  Field3D & operator=(const Field3D &rhs);
  Field3D & operator=(const Field2D &rhs);
  Field3D & operator=(const FieldPerp &rhs);
  const bvalue & operator=(const bvalue &val);
  BoutReal operator=(const BoutReal val);

  /// Addition operators
  Field3D & operator+=(const Field3D &rhs);
  Field3D & operator+=(const Field2D &rhs);
  Field3D & operator+=(const FieldPerp &rhs);
  Field3D & operator+=(const BoutReal &rhs);
  
  /// Subtraction
  Field3D & operator-=(const Field3D &rhs);
  Field3D & operator-=(const Field2D &rhs);
  Field3D & operator-=(const FieldPerp &rhs);
  Field3D & operator-=(const BoutReal &rhs);

  /// Multiplication
  Field3D & operator*=(const Field3D &rhs);
  Field3D & operator*=(const Field2D &rhs);
  Field3D & operator*=(const BoutReal &rhs);
  
  /// Division
  Field3D & operator/=(const Field3D &rhs);
  Field3D & operator/=(const Field2D &rhs);
  Field3D & operator/=(const BoutReal &rhs);
  
  // Binary operators

  const Field3D operator+() const {return *this;}
  const FieldPerp operator+(const FieldPerp &other) const;
  const Field3D operator+(const BoutReal &rhs) const;

  const Field3D operator-() const;
  const Field3D operator-(const Field3D &other) const;
  const Field3D operator-(const Field2D &other) const;
  const FieldPerp operator-(const FieldPerp &other) const;
  const Field3D operator-(const BoutReal &rhs) const;

  const Field3D operator*(const Field3D &other) const;
  const Field3D operator*(const Field2D &other) const;
  const FieldPerp operator*(const FieldPerp &other) const;
  const Field3D operator*(const BoutReal rhs) const;

  const Field3D operator/(const Field3D &other) const;
  const Field3D operator/(const Field2D &other) const;
  const FieldPerp operator/(const FieldPerp &other) const;
  const Field3D operator/(const BoutReal rhs) const;

  // Stencils for differencing

  /// Takes a location and fills values in the stencil
  /*
  void setStencil(bstencil *fval, bindex *bx) const {
    setStencil(fval, bx, true);
  }
  void setStencil(bstencil *fval, bindex *bx, bool need_x) const;
  */

  void setXStencil(stencil &fval, const bindex &bx, CELL_LOC loc = CELL_DEFAULT) const;
  void setXStencil(forward_stencil &fval, const bindex &bx, CELL_LOC loc = CELL_DEFAULT) const;
  void setXStencil(backward_stencil &fval, const bindex &bx, CELL_LOC loc = CELL_DEFAULT) const;
  void setYStencil(stencil &fval, const bindex &bx, CELL_LOC loc = CELL_DEFAULT) const;
  void setYStencil(forward_stencil &fval, const bindex &bx, CELL_LOC loc = CELL_DEFAULT) const;
  void setYStencil(backward_stencil &fval, const bindex &bx, CELL_LOC loc = CELL_DEFAULT) const;
  void setZStencil(stencil &fval, const bindex &bx, CELL_LOC loc = CELL_DEFAULT) const;

  /// Shifts specified points by angle
  void shiftZ(int jx, int jy, double zangle); 
  /// Shift all points in z by specified angle
  const Field3D shiftZ(const Field2D zangle) const; 
  const Field3D shiftZ(const BoutReal zangle) const;
  /// Shifts to/from BoutReal-space (using zShift global variable)
  const Field3D shiftZ(bool toBoutReal) const; 
  /// virtual function to shift between BoutReal and shifted space
  void shiftToReal(bool toBoutReal) {
    *this = shiftZ(toBoutReal);
  }
  
  // Slicing

  // NOTE: No shifting done in z for x array
  void getXArray(int y, int z, rvec &xv) const;
  void getYArray(int x, int z, rvec &yv) const;
  void getZArray(int x, int y, rvec &zv) const;

  void setXArray(int y, int z, const rvec &xv);
  void setYArray(int x, int z, const rvec &yv);
  void setZArray(int x, int y, const rvec &zv);

  /// Take a slice through the data at constant y
  const FieldPerp slice(int y) const;

  // FieldData virtual functions
  
  bool isReal() const   { return true; }         // Consists of BoutReal values
  bool is3D() const     { return true; }         // Field is 3D
  int  byteSize() const { return sizeof(BoutReal); } // Just one BoutReal
  int  BoutRealSize() const { return 1; }
  int  getData(int x, int y, int z, void *vptr) const;
  int  getData(int x, int y, int z, BoutReal *rptr) const;
  int  setData(int x, int y, int z, void *vptr);
  int  setData(int x, int y, int z, BoutReal *rptr);

  bool ioSupport() { return true; } ///< This class supports I/O operations
  BoutReal *getData(int component) { 
    return block->data[0][0];
  }
  void zeroComponent(int component){
    *this = 0.0;
  }

  /// Visitor pattern support
  void accept(FieldVisitor &v) override { v.accept(*this); }
  
#ifdef CHECK
  bool checkData(bool vital = false) const; ///< Checks if the data is all valid. 

  void doneComms() { bndry_xin = bndry_xout = bndry_yup = bndry_ydown = true; }
#else
  // Define the above functions to do nothing
  bool checkData(bool vital = false) const {}
  void doneComms() {}
#endif

  friend class Vector3D;
  
  static void cleanup(); // Frees all memory

  void setBackground(const Field2D &f2d); // Boundary is applied to the total of this and f2d
  void applyBoundary(bool init=false);
  void applyBoundary(BoutReal t);
  void applyBoundary(const string &condition);
  void applyBoundary(const char* condition) { applyBoundary(string(condition)); }
  void applyBoundary(const string &region, const string &condition);
  void applyTDerivBoundary();
  void setBoundaryTo(const Field3D &f3d); ///< Copy the boundary region
  
 private:
  /// Boundary - add a 2D field
  const Field2D *background;
  
  /// Interpolates in z using up to 4 points
  BoutReal interpZ(int jx, int jy, int jz0, BoutReal zoffset, int order) const;

  // NOTE: Data structures mutable, though logically const

  /// Data block for this object
  mutable memblock3d *block;

  /// Number of blocks allocated
  static int nblocks;
  /// Linked list of all memory blocks
  static memblock3d *blocklist;
  /// Linked list of free blocks
  static memblock3d *free_block;

  /// Get a new block of data, either from free list or allocate
  memblock3d* newBlock() const;
  /// Makes sure data is allocated and only referenced by this object
  void allocData() const;
  /// Releases the data array, putting onto global stack
  void freeData();
  
  CELL_LOC location; // Location of the variable in the cell
  
  Field3D *deriv; ///< Time derivative (may be NULL)

  /// FCI method
  Field3D *yup_field, *ydown_field;
};

// Non-member overloaded operators

const Field3D operator-(const BoutReal &lhs, const Field3D &rhs);


Field3D operator+(Field3D lhs, const Field3D &other);
Field3D operator+(Field3D lhs, const Field2D &other);
Field3D operator+(Field3D lhs, BoutReal rhs);
Field3D operator+(BoutReal lhs, Field3D rhs);
const Field3D operator*(const BoutReal lhs, const Field3D &rhs);
const Field3D operator/(const BoutReal lhs, const Field3D &rhs);

// Non-member functions
BoutReal min(const Field3D &f, bool allpe=false);
BoutReal max(const Field3D &f, bool allpe=false);

Field3D pow(const Field3D &lhs, const Field3D &rhs);
Field3D pow(const Field3D &lhs, const Field2D &rhs);
Field3D pow(const Field3D &lhs, const FieldPerp &rhs);
Field3D pow(const Field3D &f, BoutReal rhs);
Field3D pow(BoutReal lhs, const Field3D &rhs);

const Field3D SQ(const Field3D &f);
const Field3D sqrt(const Field3D &f);
const Field3D abs(const Field3D &f);

const Field3D exp(const Field3D &f);
const Field3D log(const Field3D &f);

const Field3D sin(const Field3D &f);
const Field3D cos(const Field3D &f);
const Field3D tan(const Field3D &f);

const Field3D sinh(const Field3D &f);
const Field3D cosh(const Field3D &f);
const Field3D tanh(const Field3D &f);

bool finite(const Field3D &var);

const Field3D copy(const Field3D &f);

const Field3D floor(const Field3D &var, BoutReal f);

const Field3D filter(const Field3D &var, int N0);
const Field3D lowPass(const Field3D &var, int zmax);
const Field3D lowPass(const Field3D &var, int zmax, int zmin);

Field2D DC(const Field3D &f);

/*!
 * @brief Returns a reference to the time-derivative of a field
 * 
 * Wrapper around member function f.timeDeriv()
 *
 */
inline Field3D& ddt(Field3D &f) {
  return *(f.timeDeriv());
}

#endif /* __FIELD3D_H__ */
