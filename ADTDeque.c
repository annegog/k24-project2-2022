///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Vector μέσω Dynamic Array.
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>

#include "ADTDeque.h"

// Το αρχικό μέγεθος που δεσμεύουμε
#define DEQUE_MIN_CAPACITY 10

struct deque_node {
	Pointer value;
};

struct deque {
	DequeNode array;
	int size;
	int capacity;
	DestroyFunc destroy_value;
};


Deque deque_create(int size, DestroyFunc destroy_value) {
	Deque deq = malloc(sizeof(*deq));
	deq->size = size;
	deq->destroy_value = destroy_value;

	// Δέσμευση μνήμης για τον πίνακα. Αρχικά δεσμεύουμε xώρο για τουλάχιστον DEQUE_MIN_CAPACITY
	deq->capacity = size < DEQUE_MIN_CAPACITY ? DEQUE_MIN_CAPACITY : size;
	deq->array = calloc(deq->capacity, sizeof(*deq->array));		// αρχικοποίηση σε 0 (NULL)

	return deq;
}

int deque_size(Deque deque) {
	return deque->size;
}

Pointer deque_get_at(Deque deque, int pos) {
	assert(pos >= 0 && pos < deque->size);

	return deque->array[pos].value;
}

void deque_set_at(Deque deque, int pos, Pointer value) {
	assert(pos >= 0 && pos < deque->size);

	// Αν υπάρχει συνάρτηση destroy_value, την καλούμε για το στοιχείο που αντικαθίσταται
	if (value != deque->array[pos].value && deque->destroy_value != NULL)
		deque->destroy_value(deque->array[pos].value);

	deque->array[pos].value = value;
}

void deque_insert_first(Deque deque, Pointer value){
	if (deque->capacity == deque->size) {
		deque->capacity *= 2;
		deque->array = realloc(deque->array, deque->capacity * sizeof(*deque->array));
	}
	// μετακινούνται τα υπόλοιπα μια θέση 
    for(int i = deque->size -1; i >= 0; i--){
        deque->array[i+1].value = deque->array[i].value;
    }
    deque->array[0].value = value;
    deque->size++;
}

void deque_insert_last(Deque deque, Pointer value) {
	// Μεγαλώνουμε τον πίνακα ώστε να χωράει τουλάχιστον size στοιχεία
	// Διπλασιάζουμε κάθε φορά το capacity
    if (deque->capacity == deque->size) {
		// Προσοχή: δεν πρέπει να κάνουμε free τον παλιό pointer, το κάνει η realloc
		deque->capacity *= 2;
		deque->array = realloc(deque->array, deque->capacity * sizeof(*deque->array));
	}

	// Μεγαλώνουμε τον πίνακα και προσθέτουμε το στοιχείο
	deque->array[deque->size].value = value;
	deque->size++;
}

void deque_remove_last(Deque deque) {
	assert(deque->size != 0);		// LCOV_EXCL_LINE

	// Αν υπάρχει συνάρτηση destroy_value, την καλούμε για το στοιχείο που αφαιρείται
	if (deque->destroy_value != NULL)
		deque->destroy_value(deque->array[deque->size - 1].value);

	// Αφαιρούμε στοιχείο οπότε ο πίνακας μικραίνει
	deque->size--;

	// Μικραίνουμε τον πίνακα αν χρειαστεί, ώστε να μην υπάρχει υπερβολική σπατάλη χώρου.
	// Για την πολυπλοκότητα είναι σημαντικό να μειώνουμε το μέγεθος στο μισό, και μόνο
	// αν το capacity είναι τετραπλάσιο του size (δηλαδή το 75% του πίνακα είναι άδειος).
	//
	if (deque->capacity > deque->size * 4 && deque->capacity > 2*DEQUE_MIN_CAPACITY) {
		deque->capacity /= 2;
		deque->array = realloc(deque->array, deque->capacity * sizeof(*deque->array));
	}
}

void deque_remove_first(Deque deque){
    assert(deque->size != 0);

	if (deque->destroy_value != NULL)
		deque->destroy_value(deque->array[0].value);

	// Αφαιρούμε στοιχείο οπότε ο πίνακας μικραίνει
	deque->size--;

    for(int i= 0; i < deque->size; i++){
        deque->array[i] = deque->array[i+1];
    }

	// Μικραίνουμε τον πίνακα αν χρειαστεί, ώστε να μην υπάρχει υπερβολική σπατάλη χώρου.
	if (deque->capacity > deque->size * 4 && deque->capacity > 2*DEQUE_MIN_CAPACITY) {
		deque->capacity /= 2;
		deque->array = realloc(deque->array, deque->capacity * sizeof(*deque->array));
	}
}

Pointer deque_find(Deque deque, Pointer value, CompareFunc compare) {
	// Διάσχιση του vector
	for (int i = 0; i < deque->size; i++)
		if (compare(deque->array[i].value, value) == 0)
			return deque->array[i].value;
	return NULL;
}

DestroyFunc deque_set_destroy_value(Deque deque, DestroyFunc destroy_value) {
	DestroyFunc old = deque->destroy_value;
	deque->destroy_value = destroy_value;
	return old;
}

void deque_destroy(Deque deque) {
	if (deque->destroy_value != NULL)
		for (int i = 0; i < deque->size; i++)
			deque->destroy_value(deque->array[i].value);

	// Πρέπει να κάνουμε free τόσο τον πίνακα όσο και το struct
	free(deque->array);
	free(deque);
}


// Συναρτήσεις για διάσχιση μέσω node /////////////////////////////////////////////////////

DequeNode deque_first(Deque deque) {
	if (deque->size == 0)
		return DEQUE_BOF;
	else	
		return &deque->array[0];
}

DequeNode deque_last(Deque deque) {
	if (deque->size == 0)
		return DEQUE_EOF;
	else
		return &deque->array[deque->size-1];
}

DequeNode deque_next(Deque deque, DequeNode node) {
	if (node == &deque->array[deque->size-1])
		return DEQUE_EOF;
	else
		return node + 1;
}

DequeNode deque_previous(Deque deque, DequeNode node) {
	if (node == &deque->array[0])
		return DEQUE_EOF;
	else
		return node - 1;
}

Pointer deque_node_value(Deque deque, DequeNode node) {
	return node->value;
}

DequeNode deque_find_node(Deque deque, Pointer value, CompareFunc compare) {
	// Διάσχιση του vector
	for (int i = 0; i < deque->size; i++)
		if (compare(deque->array[i].value, value) == 0)
			return &deque->array[i];		// βρέθηκε

	return DEQUE_EOF;
}