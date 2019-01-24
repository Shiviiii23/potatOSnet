
from __future__ import print_function
#Step 1: making all the necessary imports

import math

# from sklearn import metrics
import tensorflow as tf
from tensorflow.python.data import Dataset

import numpy as np
import pandas as pd
import tensorflow as tf

# print("MY FIRST MACHINE LEARNING PROJECT:" +
#     "This project will look at data for red wine (different chemical" +
#     "aspects of the wine) and use"+
#     "a linear regressor to predict the quality of the wine" +)
#Step 2: obtaining the data and reindexing it so it will be random
winequality_data = pd.read_csv("/Users/shivirevuru/Downloads/winequality/winequality-red.csv", sep=';',
    encoding = 'utf-8-sig')
print (winequality_data.head(10))

#winequality_data = winequality_data.reindex(np.random.permutation(winequality_data))
#print (winequality_data.head(10))


#defining features
#Note to self: are you sure these are the features you want to use?
#Notes: this is different from neural networks, because NN use a COMBINATION (like feature crosses)
# of features
print("-------------KEYS-----------")
print(winequality_data.keys())

processed_targets = pd.DataFrame()
processed_features = winequality_data[['density', 'pH', 'chlorides', 'alcohol', 'sulphates', 'fixed_acidity',
'volatile_acidity', 'citric_acid', 'residual_sugar', 'free_sulfur_dioxide', 'total_sulfur_dioxide']].copy()

processed_targets[['quality']] = winequality_data[['quality']]

#with pd.option_context ('display.max_rows', None, 'display.max_columns', None):
#print (winequality_data)

#setting our training and testing examples
training_examples = processed_features.head(1200)
training_targets = processed_targets.head(1200)
validation_examples = processed_features.tail(399)
validation_targets = processed_targets.tail(399)

#helper method 1: will construct a TensorFlow feature column
def create_column(input_features):
    return set([tf.feature_column.numeric_column(my_feature) for my_feature in input_features])

#helper method 2: will return a tuple of features, lables, for next data batch
#(this function will eventually be put in a loop) the function uses an iterator (getnext)
def my_input_func(features, targets, batch_size =1, shuffle = True, num_epochs = None ):

    #pandas data is converted into a dict of num py arrays
    features = {key: np.array(value) for key, value in dict(features).items()}
    dataset = Dataset.from_tensor_slices((features, targets))
    #combines consecutive elements of dataset into batches
    dataset = dataset.batch(batch_size).repeat(num_epochs) #repeats dataset "num_epoch" number of times
    if shuffle:
        dataset.shuffle(10000)
    features, labels = dataset.make_one_shot_iterator().get_next()
    return features, labels




#now, we define a function to actually build and train a model
#pseudocode:
#define function to take in the inputs: learning rate, steps, batch size, feature columns,
#training examples, training targets, validation examples, and validation targets

#returns a linear regressor object trained on the training data
def training(learning_rate,
         steps,
         batch_size,
         feature_columns,
         training_examples,
         training_targets,
         validation_examples,
         validation_targets):


    # use the helper functions to "preprocess" the data to feed in
    # to the classifier
    #step 1: create a linear regressor object
    #first, pick an optimizer (I will use the general gradientdescentoptimizer)
    shivi_optimizer = tf.train.GradientDescentOptimizer(learning_rate = learning_rate)
    shivi_optimizer = tf.contrib.estimator.clip_gradients_by_norm(shivi_optimizer, 5.0)
    linear_regressor = tf.estimator.LinearRegressor(feature_columns = create_column(training_examples),
                                                    optimizer = shivi_optimizer)



    #now, get the data in the right format - make it iterable/non iterable

    my_training_input = lambda: my_input_func(training_examples, training_targets,
                                              batch_size = batch_size)

    my_predictions_training = lambda: my_input_func(training_examples,
                                                    training_targets,
                                                    num_epochs = 1) #data won't be repeated

    my_validation_training = lambda: my_input_func(validation_examples,
                                                   validation_targets,
                                                   num_epochs=1)

    num_times = 10
    steps_per_time = steps/num_times


    for x in range(num_times):
        linear_regressor.train(input_fn = my_training_input, #this will iterate through and find the
                               #next input
                               steps = steps_per_time)

    #compute predictions
        training_predictions = linear_regressor.predict(input_fn = my_predictions_training)
    #turn it into an array of predictions
        training_predictions = np.array([item['predictions'][0] for item in training_predictions])



    #do the same thing for the vaidation data
        val_predictions = linear_regressor.predict(input_fn = my_validation_training)

        val_predictions = np.array([item['predictions'][0] for item in val_predictions])

        # evall = evaluate(input_fn = my_predictions_training)

    #print out that it is training
        print('Training the model on period: ', x+1)
        # print("Accuracy: $0.2f" % eval['accuracy'])
    print("training finished!")

    #finally, return a linear regressor that is trained on data
    return linear_regressor

#call training function
classifier = training(
    learning_rate = .0001,
    steps = 100,
    batch_size = 10,
    feature_columns = None,
    training_examples = training_examples,
    training_targets = training_targets,
    validation_examples= validation_examples,
    validation_targets = validation_targets)

my_validation_training = lambda: my_input_func(validation_examples,
                                                   validation_targets,
                                                   num_epochs=1)
validation_prediction = classifier.predict(input_fn = my_validation_training)
validation_prediction = np.array([item['predictions'][0] for item in validation_prediction])

print(validation_prediction)

#make sure this is right
my_predictions_training = lambda: my_input_func(training_examples,
                                                    training_targets,
                                                    num_epochs = 1)


#calculate accuracy
eval = classifier.evaluate(input_fn = my_predictions_training)
print(eval.keys())
print("Loss: %0.2f" % eval['loss'])
print("Average Loss : %0.2f" % eval['average_loss'])
