/**
 * @file cobj.h
 * @brief Header file for managing dynamic creation and destruction of objects in a game.
 *
 * This file defines the CObj class, which is used to manage the dynamic creation and
 * destruction of objects, such as those found in games. It provides an interface for
 * handling object lifecycle, including step processing, drawing, touch handling,
 * and priority-based management.
 *
 * The CObj class includes mechanisms to handle object identifiers, priority levels,
 * and type identification, allowing for organized and efficient management of
 * game objects.
 *
 * Usage:
 * @code
 * #include <cobj.h>
 * 
 * // Reset object holder
 * CObjHolder_Reset();
 * 
 * // Create and register a new object
 * MyCObj* obj = new MyCObj();
 * HObj hObj = CObjHolder_Entry(obj, priority);
 * 
 * // Perform game loop operations
 * while (gameRunning) {
 *     CObjHolder_Step(cmd);
 * }
 * 
 * // Remove and delete the object
 * CObjHolder_Remove(hObj);
 * delete obj;
 * @endcode
 */
#pragma once
#include <b8/type.h>
#include <b8/assert.h>
#include <b8/ppu.h>
#include <b8/misc.h>
#include <handle.h>
#include <vector>

typedef unsigned long HObj;
class CObj{
  friend  void  CObjHolder_Step( b8PpuCmd* cmd_ );

  // id
  private:  u32 _id_cobj;
  /**
   * @brief Get the ID of the object.
   * @return The ID of the object.
   */
  public:   u32 GetId() const {
    return  _id_cobj;
  }

  // step
  private:  u32 _cnt_step_called = 0;
  private:  virtual void  vOnStep() = 0;
  private:   void  Step();

  // touch
  private:  virtual void  vOnTouch(){}
  private:  void  Touch();

  // draw
  private:  virtual void  vOnDraw( b8PpuCmd* cmd_ ){
    UNUSED(cmd_);
  }
  private:  void  Draw( b8PpuCmd* cmd_ );

  // handle
  private:  HObj  _hObj = HANDLE_NULL;
  /**
   * @brief Get the handle of the object.
   * @return The handle of the object.
   */
  public:   HObj  GetHandle() const {
    return _hObj;
  }

  /**
   * @brief Set the handle of the object.
   * @param hObj_ The handle to set.
   */
  public: void  SetHandle( HObj hObj_ ){
    _hObj = hObj_;
  }

  // priority
  private:  u32  _priority = 0;
  /**
   * @brief Set the priority of the object.
   * @param priority_ The priority to set.
   */
  public:   void  SetPriority( u32 priority_ ){
    _priority = priority_;
  }
  /**
   * @brief Get the priority of the object.
   * @return The priority of the object.
   */
  public:   u32   GetPriority() const {
    return  _priority;
  }

  // kill
  private:    bool _req_kill = false;
  /**
   * @brief Request to kill the object.
   */
  public:     void  ReqKill(){
    _req_kill = true;
  }
  /**
   * @brief Check if the object is requested to be killed.
   * @return True if the object is requested to be killed, false otherwise.
   */
  public:     bool  IsReqKill() const {
    return _req_kill;
  }

  // type of
  private:    u32 _type_id = 0x00000000;
  /**
   * @brief Set the type ID of the object.
   * @param type_id_ The type ID to set.
   */
  protected:  void  SetTypeId( u32 type_id_ ){
    _type_id = type_id_;
  }
  /**
   * @brief Get the type ID of the object.
   * @return The type ID of the object.
   */
  public:     u32  GetTypeId(){
    return  _type_id;
  }
  /**
   * @brief Check if the object is of a specific type.
   * @param type_id_ The type ID to check.
   * @return True if the object is of the specified type, false otherwise.
   */
  public:     bool IsTypeOf( u32 type_id_ ){
    return _type_id == type_id_ ? true : false;
  }

  public:
    CObj();
    virtual ~CObj();
};

extern  void  CObjHolder_Reset();
extern  HObj  CObjHolder_Entry( CObj* obj , u32 priority );
extern  void  CObjHolder_Remove( HObj hObj );
extern  void  CObjHolder_Enum( std::vector< HObj >& dest_ , u32 prio_ , u32 type_id_ );
extern  void  CObjHolder_Step( b8PpuCmd* cmd_ );
extern  void  CObjHolder_Pause( s32 cnt_pause_ );
extern  CObj* cobj( HObj hObj );
template <class T>
class Handle {
  HObj  _h = HANDLE_NULL;
  T*    _p = nullptr;

  void  _DynamicCast(){
    _p = dynamic_cast< T* >( cobj( _h ) );
    _ASSERT( _p , "failed dynamic_cast" );
  }

public:
  /**
   * @brief Arrow operator to access the underlying object.
   * @return Pointer to the underlying object.
   */
  inline	T* operator -> (){
    if( nullptr == _p ){
      _ASSERT( 0 , "failed dynamic_cast" );
    }
    return _p;
  }

  /**
   * @brief Assignment operator for handle.
   * @param h_ Handle to assign.
   * @return Reference to the current object.
   */
  Handle<T> operator = ( HObj h_ ){
    _h = h_;
    _DynamicCast();
    return *this;
  }

  /**
   * @brief Assignment operator for another handle.
   * @param x_ Handle to assign.
   * @return Reference to the current object.
   */
  Handle<T> operator = ( Handle<T> x_ ){
    _h = x_._h;
    _DynamicCast();
    return *this;
  }

  /**
   * @brief Constructor with handle parameter.
   * @param hObj_ Handle to assign.
   */
  Handle( HObj hObj_ )
  {
    _h = hObj_;
    _DynamicCast();
  }

  Handle(){}
};
