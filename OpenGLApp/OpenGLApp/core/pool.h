#ifndef POOL_H
#define POOL_H

#include <vector>
#include <memory>
#include <utility>

//tipo generico, per poter creare pool per oggetti di tipo diverso
template <typename T>

class ObjectPool {
private:
	//vettore in cui vengono inseriti gli oggetti
	std::vector<std::unique_ptr<T>> pool;
public:
	template <typename... Args>
	ObjectPool(int startingSize, Args&&... args) {
		for (int i = 0; i < startingSize; i++) {
			pool.push_back(std::make_unique<T>(std::forward<Args>(args)...));
			//printf("aggiunto un oggetto alla pool\n");
		}
	}

	//per prendere un oggetto dalla pool
	T* getInstance() {
		//se sono finiti gli oggetti nella pool ne creo uno nuovo e ritorno quello
		if (pool.empty()) {
			return nullptr;
		}

		//altrimenti ne prendo uno dalla pool
		std::unique_ptr<T> instance = std::move(pool.back());
		pool.pop_back();
		//printf("sto prendendo un oggetto dalla pool\n");
		return instance.release();
	}

	void returnToPool(T* instance) {
		pool.emplace_back(instance);
	}
	
	





};

#endif