#pragma once

//These macros are used to generate getters for required fields that the component needs
//and that are stored in another class (usually the child or another component)
#define COMP_CROSS_VIRTUAL(comp_name, var_type, var_name) \
    virtual var_type _##comp_name##_##var_name() const = 0;

#define COMP_CROSS_VARIABLE(comp_name, var_type, var_name) \
    virtual var_type _##comp_name##_##var_name() const {return m_##var_name;}

#define COMP_CROSS_VARIABLE_PUBLIC(comp_name, var_type, var_name) \
    virtual var_type _##comp_name##_##var_name() const {return ##var_name;}

//@WIP: Maybe we can create a MACRO to set some flags in child classes to 
//check which components an entity has so we don't have to use dynamic_cast
//Is it needed? Is dynamic cast that slow??
