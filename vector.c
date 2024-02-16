// Vector.c
// Gabriel Cadieux
// Last Modified: 9/12/23
// Vector.c contains all of code needed to mimic a vector
// Note: My Vector.c contains a vector_print function I used for testing, I commented it out so it doesn't cause issues

#include "vector.h"
#include <stdlib.h>


typedef struct Vector {
    int size;
    int capacity;
    int64_t *values;
} Vector;


// Allocation/Creation of new vector functions

// Creates new vector with no initialized capacity
Vector *vector_new(void) 
{
    Vector *v = (Vector *)malloc(sizeof(Vector));
    v->size = 0;
    v->capacity = 0;
    v->values = (int64_t *)calloc(v->capacity, sizeof(int64_t));
    return v;
}

// Creates new vector with specified capacity
Vector *vector_new_with_capacity(int capacity)
{
    Vector *v = (Vector *)malloc(sizeof(Vector));
    v->size = 0;
    v->capacity = capacity;
    v->values = (int64_t *)calloc(v->capacity, sizeof(int64_t));
    return v;
}

// Frees our vec-values memory and our vec memory
// Should take care of the malloc and calloc used to create this memory
void vector_free(Vector *vec) 
{
    free(vec->values);
    free(vec);
}

// resizes the vector, if our new size exceeds our capacity we must raise the capacity and reallocate the vector
// See vector_reserve's comments on the breakdown of the reallocation since the method is the same
void vector_resize(Vector *vec, int new_size)
{
    vec->size = new_size;
    if (new_size > vec->capacity) 
    {
        vec->capacity = new_size;
        int64_t *tempValues;
        tempValues = (int64_t *)calloc(vec->capacity, sizeof(int64_t));
        for (int i = 0; i < vec->size; i++) {
            tempValues[i] = vec->values[i];
        }
        free(vec->values);
        vec->values = tempValues;
        
    }
}


// Reserves more or less memory for our vector
void vector_reserve(Vector *vec, int new_capacity)
{
    // Capacity will always equal the new capacity
    vec->capacity = new_capacity;

    // If new capacity is less than our current size then shrink the size down to the new capacity
    if (new_capacity < vec->size) 
    {
        vec->capacity = vec->size;
    }

    // else the new capacity is >= to the size and we will reallocate the vector
    else 
    {
        // Used a temporary int64_t pointer to copy the values from vec->values, then free the old vec->values
        // and set it to point to where our temp pointer is pointing to, temp pointer gets destroyed when going out of scope
        vec->capacity = new_capacity;
        int64_t *tempValues;
        tempValues = (int64_t *)calloc(vec->capacity, sizeof(int64_t));
        for (int i = 0; i < vec->size; i++) {
            tempValues[i] = vec->values[i];
        }
        free(vec->values);
        vec->values = tempValues;
    }
}

// Element Functions

// Pushes value onto the end of the vector, if we are less than our capacity, add the value and increment size
// Else, we need to resize the vector by 1 to increase the capacity by 1 and then insert the value where it needs to be
void vector_push(Vector *vec, int64_t value)
{
    
    if (vec->size < vec->capacity) 
    {
        vec->values[vec->size] = value;
        vec->size++;
    }
    else 
    {
        vector_resize(vec, vec->size + 1);
        vec->values[vec->size - 1] = value;
    }
    
}

// Inserts given value at given index, then moves everything to the right by 1 if it was not placed at the end
void vector_insert(Vector *vec, int index, int64_t value)
{
    if ((index > -1) && (index < vec->size)) {

        // Make some temp variables to help move values to the right
        int64_t tempValue1;
        int64_t tempValue2;
        bool flag = true;

        // Increase vector size since we are adding a variable
        vector_resize(vec, (vec->size + 1));

        // Save value at initial location and load in given value
        tempValue1 = vec->values[index];
        vec->values[index] = value;

        // This for loop allows us to flip between using two differnt tempvalue holders since I need two to be able to
        // simultaneously save values and load values into their appropiate spots without overwriting or losing values
        for (int i = index + 1; i < vec->size; i++) {
            if (flag) {
                tempValue2 = vec->values[i];
                vec->values[i] = tempValue1;
                flag = false;
            }
            else {
                tempValue1 = vec->values[i];
                vec->values[i] = tempValue2;
                flag = true;
            }
            
        }
    }
    else if (index > vec->size) {
        vector_push(vec, value);
    }
    else {
        // Do Nothing, out of bounds
    }
}

// Removes value at given index and shifts all values afterwards to the left (decrements size by 1 also)
bool vector_remove(Vector *vec, int index)
{
    // Check our bounds, then we can simply set the next value in the vector [i + 1] to our current index to shift everything left by 1
    if ((index > -1) && (index < vec->size)) {
        for (int i = index; i < vec->size; i++) {
            vec->values[i] = vec->values[i + 1];
        }
        vector_resize(vec, vec->size - 1);
        return true;
    }
    else {
        return false;
    }
}

// Gets a value at the given index in the vector, this simply returns true or false if we could successfully retrieve the value
// Must use a variable from main.c to see what the value was
bool vector_get(Vector *vec, int index, int64_t *value)
{
    if  ((index > -1) && (index < vec->size))
    {
        *value = vec->values[index];
        return true;
    }
    else {
        return false;
    }
}

// Sets a value at the given index in the vector, this does not move other values like insert does
bool vector_set(Vector *vec, int index, int64_t value)
{
    // Check the given index is in bounds, then proceed with setting the value if it is
    if ((index > -1) && (index < vec->size))
    {
        vec->values[index] = value;
        return true;
    }
    else {
        return false;
    }
    

}

// Reset Vector's size to 0
void vector_clear(Vector *vec)
{
    vec->size = 0;
}

// Finding/Sorting

// Checks if given value is in our vector by simply iterating through the vector and checking each element (time complexity O(n))
int  vector_find(Vector *vec, int64_t value)
{
    for (int i = 0; i < vec->size; i++) 
    {
        if (vec->values[i] == value) 
        {
            return i;
        }
        else {
            continue;
        }
    }
    return -1;
}

static bool comp_ascending(int64_t left, int64_t right)
{
    // Consider left <= right to mean left is in place with right. Meaning
    // we swap if left > right.
    return left <= right;
}

void vector_sort(Vector *vec)
{
    vector_sort_by(vec, comp_ascending);
}

void vector_sort_by(Vector *vec, bool (*comp)(int64_t left, int64_t right))
{
    int i;
    int j;
    int min;
    int64_t tmp;

    for (i = 0; i < vec->size - 1; i++) {
        min = i;
        for (j = i + 1; j < vec->size; j++) {
            if (!comp(vec->values[min], vec->values[j])) {
                min = j;
            }
        }
        if (min != i) {
            tmp = vec->values[min];
            vec->values[min] = vec->values[i];
            vec->values[i]   = tmp;
        }
    }
}


// Conducts Binary Search on the Vector assuming it has been sorted already (time complexity O(logn))
int  vector_bsearch(Vector *vec, int64_t value)
{
    // Set initial high/low
    int low = 0;
    int high = vec->size;
    
    // While true loop works here since we will either break it by returning that we found
    // the value we were looking for or return -1 if we didn't
    while (true)
    {
        // Calculate middle each iteration with the following equation
        int middle = low + (high - low)/2;

        // If our middle index equals the value we are looking for, return the current index (middle)
        if (vec->values[middle] == value) {
            return middle;
        }

        // Else if our low equals our high, or high goes below our low, or low goes above our high,
        // We did not find the value, return -1
        else if ((low == high) || (high < low)) {
            return -1;
        }

        // Else, check if wanted value is lower or higher than current value, update low/high accordingly
        else {
            if (value > vec->values[middle]) {
                low = middle + 1;
            }
            else {
                high = middle - 1;
            }
        }
    }
    
    

}

// Accessors for fields in the vector

// This print function was used for testing
/*void vector_print(Vector *vec) 
{
    for (int i = 0; i < vec->size; i++) 
    {
        if (vec->size == (i + 1)) 
        {
            printf("%ld\n", vec->values[i]);
        }
        else 
        {
            printf("%ld ", vec->values[i]);
        }
        
    }
}*/

// Returns Vector's Capacity
int  vector_capacity(Vector *vec)
{
    return vec->capacity;
}

// Returns Vector's Size
int  vector_size(Vector *vec)
{
    return vec->size;
}

