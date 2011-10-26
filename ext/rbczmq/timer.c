#include <rbczmq_ext.h>

static VALUE intern_call;

static void rb_czmq_mark_timer(void *ptr)
{
    zmq_timer_wrapper *timer = ptr;
    rb_gc_mark(timer->callback);
}

static void rb_czmq_free_timer(void *ptr)
{
    zmq_timer_wrapper *timer = ptr;
    if (timer) xfree(timer);
}

VALUE rb_czmq_timer_s_new(int argc, VALUE *argv, VALUE timer)
{
    VALUE delay, times, proc, callback;
    size_t timer_delay;
    zmq_timer_wrapper *tr = NULL;
    rb_scan_args(argc, argv, "21&", &delay, &times, &proc, &callback);
    if (NIL_P(proc) && NIL_P(callback)) rb_raise(rb_eArgError, "no callback given!");
    if (NIL_P(proc)) {
        rb_need_block();
    } else {
        callback = proc;
    }
    if (TYPE(delay) != T_FIXNUM && TYPE(delay) != T_FLOAT) rb_raise(rb_eTypeError, "wrong delay type %s (expected Fixnum or Float)", RSTRING_PTR(rb_obj_as_string(delay)));
    Check_Type(times, T_FIXNUM);
    timer_delay = (size_t)(((TYPE(delay) == T_FIXNUM) ? FIX2LONG(delay) : RFLOAT_VALUE(delay)) * 1000); 
    timer = Data_Make_Struct(rb_cZmqTimer, zmq_timer_wrapper, rb_czmq_mark_timer, rb_czmq_free_timer, tr);
    tr->delay = timer_delay;
    tr->times = FIX2INT(times);
    tr->callback = callback;
    rb_obj_call_init(timer, 0, NULL);
    return timer;
}

static VALUE rb_czmq_timer_fire(VALUE obj, VALUE args)
{
    ZmqGetTimer(obj);
    return rb_apply(timer->callback, intern_call, args); 
}

void _init_rb_czmq_timer() {
    intern_call = rb_intern("call");

    rb_cZmqTimer = rb_define_class_under(rb_mZmq, "Timer", rb_cObject);

    rb_define_singleton_method(rb_cZmqTimer, "new", rb_czmq_timer_s_new, -1);
    rb_define_method(rb_cZmqTimer, "fire", rb_czmq_timer_fire, -2);
    rb_define_alias(rb_cZmqTimer, "call", "fire");
}
