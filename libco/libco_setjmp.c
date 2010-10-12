#include "libco_ex.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../snes/debug.h"

/* By Shay Green, 2008. Public domain. */

enum { reserve_size = 4096 };
enum { reserve_fill = 0x33 };

struct cothread_t_
{
   jmp_buf context;		/* where thread last yielded */
   jmp_buf top_context;	/* prepare_thread() just before calling func() */
   void (*entry)( void );	/* user entry function, NULL if thread deleted */
   cothread_t next;		/* next in linked list */
   unsigned stack;			/* bytes allocated to stack */
   volatile char* padding;
};

/* Linked list of threads. First is always main thread. Last in chain is not a
   thread, just has jump_buf context for allocating next thread's stack. */
static struct cothread_t_ main_thread;
static cothread_t active = &main_thread; /* Currently active thread */
static volatile void* volatile force_alloc;

static void alloc_stack( cothread_t t, unsigned remain );

static int is_valid( cothread_t t )
{
   cothread_t p;
   for ( p = &main_thread; p; p = p->next )
      if ( p == t )
         return 1;
   return 0;
}

#if CO_DEBUG
static void check_padding( void )
{
   int index = 1;
   cothread_t p;
   for ( p = main_thread.next; p; p = p->next, index++ )
   {
      int i;
      assert( p->padding );
      for ( i = 0; i < reserve_size; i++ )
      {
         if ( p->padding [i] != reserve_fill )
         {
            printf( "Stack %d overflowed\n", index - 1 );
            assert( 0 );
         }
      }
   }
}
#endif

static void prepare_thread( cothread_t t )
{
   /* Function call: establish context, then return */
   if ( setjmp( t->top_context ) )
   {
      /* First switch: allocate stack, then switch back to co_create() */ 
      alloc_stack( t->next, t->stack );
      if ( !setjmp( t->top_context ) )
         longjmp( active->context, 1 );

      /* Later switches: re-establish context, then call entry() */
      while ( setjmp( t->top_context ) ) { }
      t->entry();
      assert( 0 );
   }
}

static void alloc_stack( cothread_t t, unsigned remain )
{
   volatile char reserve [reserve_size * 2];
   force_alloc = reserve; /* ensure optimizer doesn't remove this */

   SNES_DBG("alloc_stack()\n");

#if CO_DEBUG
   memset( reserve, 0, sizeof reserve );
#endif
   SNES_DBG("#1\n");

   if ( remain >= sizeof reserve )
   {
      alloc_stack( t, remain - sizeof reserve );
   }
   else
   {
      SNES_DBG("#2\n");
#if CO_DEBUG
      memset( reserve, reserve_fill, sizeof reserve );
#endif
      t->padding = reserve + reserve_size / 2;
      SNES_DBG("#3\n");
      prepare_thread( t );
   }
   SNES_DBG("Returning from alloc_stack()\n");
}

static cothread_t alloc_thread( void )
{
   cothread_t t = (cothread_t) malloc( sizeof *main_thread.next );
   if ( t )
   {
      t->next = 0;
      t->padding = 0;
   }
   return t;
}

static void alloc_next( cothread_t t )
{
   if ( !setjmp( active->context ) )
      longjmp( t->top_context, 1 );
}

cothread_t co_abi co_create( unsigned int size, void (*entry)( void ) )
{
   SNES_DBG("co_create()\n");
   SNES_DBG("#0\n");
   SNES_DBG("#0: co_main_stack_: %d\n", co_main_stack_);
   SNES_DBG("#0: co_total_stack_: %d\n", co_total_stack_);
   cothread_t t;

   /* Initialize main stack if not already */
   if ( !main_thread.next )
   {
      SNES_DBG("#1\n");
      main_thread.entry = 0;
      main_thread.next  = alloc_thread();
      SNES_DBG("#2\n");
      if ( !main_thread.next )
         return 0;

      SNES_DBG("#4: Calling alloc_stack()\n");
      alloc_stack( main_thread.next, co_main_stack_ );
      SNES_DBG("#3\n");
   }
   SNES_DBG("#4\n");

   /* Find a deleted thread with sufficient stack */
   for ( t = main_thread.next; t->next; t = t->next )
      if ( !t->entry && t->stack >= size )
         break;

   SNES_DBG("#5\n");
   /* Allocate new if necessary */
   if ( !t->next )
   {
      SNES_DBG("#6\n");
      if ( size > co_total_stack_ || !(t->next = alloc_thread()) )
         return 0;

      co_total_stack_ -= size;
      t->stack = size; /* TODO: round to multiple of 4K? */
      SNES_DBG("#7\n");
      alloc_next( t ); /* separate function to avoid GCC warning */
   }
   SNES_DBG("#8\n");

   /* Init thread */
   t->entry = entry;
   SNES_DBG("#9\n");
   memcpy( &t->context, &t->top_context, sizeof t->context );

   SNES_DBG("#10\n");
   assert( is_valid( t ) );
   SNES_DBG("Returning from co_create()\n");
   return t;
}

cothread_t co_abi co_active( void )
{
   return active;
}

void co_abi co_switch( cothread_t t )
{
#if CO_DEBUG
   assert( is_valid( t ) && t != active && (t->entry || t == &main_thread) );
   check_padding();
#endif
   if ( !setjmp( active->context ) )
   {
      active = t;
      longjmp( t->context, 1 );
   }
}

void co_abi co_delete( cothread_t t )
{
   assert( is_valid( t ) && t != active && t->entry );
   t->entry = 0;
}