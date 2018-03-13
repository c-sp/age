
#ifndef AGE_NON_COPYABLE_HPP
#define AGE_NON_COPYABLE_HPP



namespace age {

//!
//! \brief Base class for non-copyable instances.
//!
class non_copyable
{
public:

    non_copyable() {}

    non_copyable(const non_copyable&) = delete;
    non_copyable(non_copyable&&) = delete;
    non_copyable& operator=(const non_copyable&) = delete;
    non_copyable& operator=(non_copyable&&) = delete;
};

} // namespace age



#endif // AGE_NON_COPYABLE_HPP
