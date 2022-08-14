import datetime
import batch_test_general
import batch_test_network
import batch_test_allfield
import batch_test_allaircraft
import batch_test_unicode_path
import batch_test_addon



def main():
	t=[]

	t.append(datetime.datetime.now())
	batch_test_allfield.main()

	t.append(datetime.datetime.now())
	batch_test_allaircraft.main()

	t.append(datetime.datetime.now())
	batch_test_general.main()

	t.append(datetime.datetime.now())
	batch_test_network.main()

	t.append(datetime.datetime.now())
	batch_test_unicode_path.main()

	t.append(datetime.datetime.now())
	batch_test_addon.main()

	t.append(datetime.datetime.now())



	i=0
	while i<len(t)-1:
		d=datetime.timedelta.total_seconds(t[i+1]-t[i])
		print("Time for Batch ["+str(i)+"] "+str(d)+"sec")
		i=i+1


	print("Batch Test Completed.")



if __name__=="__main__":
	main()
